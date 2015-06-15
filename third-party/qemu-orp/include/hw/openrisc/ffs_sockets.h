/*
 *
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
#ifndef FFS_SOCKETS
#define FFS_SOCKETS

#define BACKLOG 10

#include <sys/socket.h>

extern char ffs_rfile_port[];
extern char ffs_wfile_port[];
extern char ffs_host_ip[];

ssize_t safe_recv(int* socket, void* buffer, size_t length, int flags);
void check_incoming_connection(int fd, int* new_fd);
void ffs_init_sockets(int* rfile_sock_fd, int* wfile_sock_fd);

#endif // FFS_SOCKETS
