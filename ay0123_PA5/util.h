/**
 * @file util.h
 * 
 * The header file defining the common data structure and function prototypes, 
 * shared by both client and server.
 * 
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
// Run client and server on the same local machine when using the following IP
//#define SERVER "127.0.0.1"
#define SERVER "129.120.151.96"

#define SYNMASK (0x0002)
#define ACKMASK (0x0010)
#define FINMASK (0x0001)

typedef struct dhcp_pkt
{
    unsigned int siaddr;         // Server IP address
    unsigned int yiaddr;         // Your IP address
    unsigned int tran_ID;        // Transaction ID
    unsigned short int lifetime; // Lease time of the IP address
} dhcp_pkt;

void die(char *s);
void prseg(dhcp_pkt *seg, char *filename);
void prmsg(const char *msg, char *filename);
void sendpkt(int sockfd, dhcp_pkt *seg, struct sockaddr_in *si);
void receive(int sockfd, dhcp_pkt *seg, struct sockaddr_in *si);

