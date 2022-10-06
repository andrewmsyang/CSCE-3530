/**
 * @file util.h
 * 
 * The header file defining the common data structure and function prototypes, 
 * shared by both client and server.
 * 
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
// #define SERVER "127.0.0.1"
#define SERVER "129.120.151.96"

#define SYNMASK (0x0002)
#define ACKMASK (0x0010)
#define FINMASK (0x0001)

// Data structure of TCP segment
typedef struct tcp_segment_t
{
    unsigned short int src;
    unsigned short int des;
    unsigned int seq;
    unsigned int ack;
    unsigned short int hdr_flags;
    unsigned short int rec;
    unsigned short int cksum;
    unsigned short int ptr;
    unsigned int opt;
    char data[1024];
} tcp_segment_t;

unsigned short checksum(tcp_segment_t *pseg);
void die(char *s);
void setsyn(tcp_segment_t *seg);
int getsyn(tcp_segment_t *seg);
void setack(tcp_segment_t *seg);
int getack(tcp_segment_t *seg);
void setfin(tcp_segment_t *seg);
int getfin(tcp_segment_t *seg);
void prseg(tcp_segment_t *seg, char *filename);
void prmsg(const char *msg, char *filename);
void populate(tcp_segment_t *seg, unsigned int src, unsigned int des);
void receive(int sockfd, tcp_segment_t *seg, struct sockaddr_in *si);

