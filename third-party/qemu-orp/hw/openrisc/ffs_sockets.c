/*
 * ffs_sockets.c  

   Copyright 2015, Google Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
  
 */

#include "hw/openrisc/ffs_sockets.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <arpa/inet.h>

static void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
static void* get_in_addr(struct sockaddr* sa)
{
    if (sa->sa_family == AF_INET) 
        return &(((struct sockaddr_in*)sa)->sin_addr);
    else return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

static void setup_port(const char* host_ip, int* sockfd, const char* port)
{
    struct addrinfo hints;
    struct addrinfo* servinfo;
    struct addrinfo* p;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;        // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP/IP stream socket

    // Get the socket addresses for a local port to listen to
    if ((rv = getaddrinfo(host_ip, port, &hints, &servinfo)) != 0) 
        { fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); exit(-1); }

    // Loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
            { perror("server: socket"); continue; }

        if (setsockopt(*sockfd , SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
            { perror("setsockopt"); exit(-1); }

        if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) 
            { close(*sockfd); perror("server: bind"); continue; }

        break;
    }

    if (p == NULL)  
        { fprintf(stderr, "server: failed to bind\n"); return; }

    freeaddrinfo(servinfo); // all done with this structure

    // Don't want accept to block
    int flags = fcntl(*sockfd, F_GETFL, 0);
    fcntl(*sockfd, F_SETFL, flags | O_NONBLOCK);

    // Listen for incoming connections
    if (listen(*sockfd, BACKLOG) == -1)
        { perror("listen"); exit(-1); }
}

// Recv doesn't block in this setup, so here we do all the error checking for recv:
// a) If we receive an actual error (i.e., not EAGAIN or EWOULDBLOCK), shut down the
//    fd and print out the error
// b) If the recv error is EAGAIN or EWOULDBLOCK, just return 0 bytes read; this isn't
//    an error, it just means there's no data present
// c) If we received 0 bytes, then the fd was closed on us, so shut it down
// d) Otherwise, just read the bytes into the buffer and return the number of bytes read
ssize_t safe_recv(int* socket, void* buffer, size_t length, int flags)
{
	ssize_t bytes = recv(*socket, buffer, length, flags);
	if (bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
		{ perror("recv"); close(*socket); *socket = -1; }
	else if (bytes == -1)
		{ bytes = 0; }
	else if (bytes == 0)
		{ close(*socket); *socket = -1; } 
	
	return bytes;
}

void check_incoming_connection(int fd, int* new_fd)
{
    if (*new_fd != -1) return;
    
    struct sockaddr_storage their_addr;
    socklen_t sin_size = sizeof(their_addr);
    char s[INET6_ADDRSTRLEN];

    *new_fd = accept(fd, (struct sockaddr *)&their_addr, &sin_size);
    if (*new_fd == -1) 
        { if (errno != EAGAIN && errno != EWOULDBLOCK) perror("accept"); return; }
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
    printf("server: got connection from %s\n", s);

    // Don't want send/recv to block
    int flags = fcntl(*new_fd, F_GETFL, 0);
    fcntl(*new_fd, F_SETFL, flags | O_NONBLOCK);
}

void ffs_init_sockets(int* rfile_sock_fd, int* wfile_sock_fd)
{
    printf("Setting up %s with rfile=%s and wfile=%s\n", ffs_host_ip, ffs_rfile_port, ffs_wfile_port);
    setup_port(ffs_host_ip, rfile_sock_fd, ffs_rfile_port);
    setup_port(ffs_host_ip, wfile_sock_fd, ffs_wfile_port);

    // Reap all dead processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) 
        { perror("sigaction"); exit(1); }
}




