#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define RFILE "9998" 
#define WFILE "9999"

#define FFS_HDR_SIZE 4 
#define FFS_DATA_SIZE 2044 

const char DATA_REQUEST = 0x01;
const char STATUS_REQUEST = 0x02;

const char CMD_START_SESSION = 0x01;
const char CMD_KILL_SESSION = 0x02;

char CHANNEL_READY[16] = {0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char CHANNEL_LAST_FAIL[16] = {0x11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char CHANNEL_LAST_SUCC[16] = {0x12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char CHANNEL_LAST_RETRY[16] = {0x13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char CHANNEL_LAST_EINPUT[16] = {0x14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

typedef struct ffs_packet_s
{
    uint16_t session;
    uint16_t reserved;
    uint8_t data[FFS_DATA_SIZE];
} ffs_packet_t;

// Turn a <session id, msg length, msg> tuple into a packet ready to be sent
static void make_packet(ffs_packet_t *pkt, uint16_t id, uint8_t *msg)
{
    pkt->session = id;
    memcpy(pkt->data, msg, FFS_DATA_SIZE);
}

// Serialize an <id, length, msg> tuple into a byte stream
static void serialize_packet(uint8_t *stream, ffs_packet_t* pkt)
{
    stream[0] = pkt->session >> 8;
    stream[1] = pkt->session;
    memcpy(stream + 4, pkt->data, FFS_DATA_SIZE);
}

// Convert a byte stream into a packet
static void deserialize_packet(ffs_packet_t *pkt, uint8_t* stream)
{
    pkt->session = (stream[0] << 8) | stream[1];
    memcpy(pkt->data, stream + 4, FFS_DATA_SIZE);
}

int connect_to(const char* host, const char* port, int* sockfd)
{
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("127.0.0.1", port, &hints, &servinfo)) != 0) 
        { fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); return -1; }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
            { perror("client: socket"); continue; }

        if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1) 
            { close(*sockfd); perror("client: connect"); continue; }

        break;
    }

    if (p == NULL) 
        { fprintf(stderr, "client: failed to connect\n"); return -1; }

    freeaddrinfo(servinfo); // all done with this structure

    return 0;
}


int main(int argc, char* argv[])
{
    int rsockfd, wsockfd, numbytes;  
    char buf[FFS_HDR_SIZE + FFS_DATA_SIZE];

    if (connect_to("localhost", RFILE, &rsockfd) != 0)
        { fprintf(stderr, "Could not open RFILE\n"); return -1; }
    else fprintf(stderr, "RFILE connected\n");

    if (connect_to("localhost", WFILE, &wsockfd) != 0)
        { fprintf(stderr, "Could not open WFILE\n"); return -1; }
    else fprintf(stderr, "WFILE connected\n");

    int session; int len;
    char msg[FFS_DATA_SIZE];;
    ffs_packet_t pkt;
    char command[8];
    char status[16] = {0};
    int i;
    while (1)
    {
        memset(msg, 0, FFS_DATA_SIZE);
        memset(buf, 0, sizeof(ffs_packet_t));
        printf("Enter command (h for help): ");
        fgets(command, 8, stdin);
        switch (command[0])
        {
            // Acknowledge receipt of last message
            case 'a':
                send(rsockfd, CHANNEL_READY, 16, 0);
                break;
            // Tell mselOS that the last message was not received properly
            case 'f':
                send(rsockfd, CHANNEL_LAST_FAIL, 16, 0);
                break;
            // Tell mselOS that the last message was not received properly
            case 'y':
                send(rsockfd, CHANNEL_LAST_RETRY, 16, 0);
                break;
            // Read a new message from the host device
            case 'r':
                send(rsockfd, &DATA_REQUEST, 1, 0);
                numbytes = recv(rsockfd, buf, FFS_HDR_SIZE + FFS_DATA_SIZE, 0);
                deserialize_packet(&pkt, buf);
                printf("Message from session %d: ", pkt.session);
                for (i = 0; i < 10; ++i)
                    printf("0x%x ", pkt.data[i]);
                printf("...\n");
                break;
            // Write a message to the host device
            case 'n':
                printf("Enter desired endpoint hash: ");
                msg[0] = CMD_START_SESSION;
                fgets(msg + 1, 32, stdin);
                msg[33] = 0;
                msg[34] = 4;    // Set port 4 -- currently ignored
                make_packet(&pkt, 0, msg);
                serialize_packet(buf, &pkt);
                send(wsockfd, &DATA_REQUEST, 1, 0);
                send(wsockfd, buf, FFS_HDR_SIZE + FFS_DATA_SIZE, 0);
                break;
            case 'k':
                memset(msg, 0, FFS_DATA_SIZE);
                sscanf(command, "k %d", &session);
                msg[0] = CMD_KILL_SESSION;
                msg[1] = session >> 8;
                msg[2] = session;
                make_packet(&pkt, 0, msg);
                serialize_packet(buf, &pkt);
                send(wsockfd, &DATA_REQUEST, 1, 0);
                send(wsockfd, buf, FFS_HDR_SIZE + FFS_DATA_SIZE, 0);
                break;
            case 'w':
                memset(msg, 0, FFS_DATA_SIZE);
                sscanf(command, "w %d", &session);
                printf("Enter packet data: ");
                fgets(msg, FFS_DATA_SIZE, stdin);
                make_packet(&pkt, session, msg);
                serialize_packet(buf, &pkt);
                send(wsockfd, &DATA_REQUEST, 1, 0);
                send(wsockfd, buf, FFS_HDR_SIZE + FFS_DATA_SIZE, 0);
                break;

            // Check the return value from the host
            case 's':
                send(wsockfd, &STATUS_REQUEST, 1, 0);
                recv(wsockfd, &status, 16, 0);
                for (i = 0; i < 16; ++i)
                    printf("0x%x ", status[i]);
                printf("\n");
                if (status[0] == CHANNEL_LAST_SUCC[0])
                    printf("OK\n");
                else if (status[0] == CHANNEL_LAST_FAIL[0])
                    printf("ERROR\n");
                else if (status[0] == CHANNEL_LAST_RETRY[0])
                    printf("RETRY\n");
                else if (status[0] == CHANNEL_READY[0])
                    printf("READY\n");
                else if (status[0] == CHANNEL_LAST_EINPUT[0])
                    printf("EINPUT\n");
                else printf("UNKNOWN RESPONSE\n");
                break;
            case 'q':
                goto EXIT;
            case 'h':
            default:
                printf("\ta   - acknowledge receipt of last message\n");
                printf("\tf   - send error to mselOS\n");
                printf("\ty   - send retry to mselOS\n");
                printf("\tr   - read next message from mselOS\n");
                printf("\tn   - start a new mselOS sessionn\n");
                printf("\tk   - kill an existing mselOS session\n");
                printf("\tw n - send msg to mselOS session n\n");
                printf("\ts   - status request from mselOS\n");
                printf("\th   - show this message\n");      
                printf("\tq   - quit\n");      
                break;
        }
    }

EXIT:
    close(rsockfd);
    close(wsockfd);

    return 0;
}
