/*
    Simple udp server
*/
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 1024 //Max length of payload buffer

// Data structure of UDP segment
typedef struct udp_segment_t
{
    unsigned short srcport;
    unsigned short dstport;
    unsigned short length;
    unsigned short checksum;
    char payload[BUFLEN];
} udp_segment_t;

// Calculate the checksum for a given segment
unsigned short checksum(udp_segment_t *pseg)
{
    unsigned short sum = 0;
    char *p = (char *)&pseg->srcport;
    for (int i = 0; i < 2; i++)
        sum += *(p++);
    p = (char *)&pseg->dstport;
    for (int i = 0; i < 2; i++)
        sum += *(p++);
    p = (char *)&pseg->length;
    for (int i = 0; i < 2; i++)
        sum += *(p++);
    p = pseg->payload;
    for (int i = 0; i < pseg->length; i++)
        sum += *(p++);
    return sum;
}

// Output to both files and console the UDP segment fields
void output(udp_segment_t *pseg, char *filename)
{
    FILE *outfile = fopen(filename, "w");
    FILE *out[] = {outfile, stdout};
    for (int i = 0; i < 2; i++)
    {
        fprintf(out[i], "Source port: %d\n", pseg->srcport);
        fprintf(out[i], "Destination port: %d\n", pseg->dstport);
        fprintf(out[i], "Length: %d\n", pseg->length);
        fprintf(out[i], "Checksum: %d\n", pseg->checksum);
        fprintf(out[i], "Payload: %s\n", pseg->payload);
    }

    fclose(outfile);
}

void die(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return 0;

    struct sockaddr_in si_me, si_other;

    int sockfd, i, slen = sizeof(si_other), recv_len;
    udp_segment_t seg;
    memset(&seg, 0, sizeof(udp_segment_t));

    //create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    // zero out the structure
    memset((char *)&si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(atoi(argv[1]));
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind socket to port
    if (bind(sockfd, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
    {
        die("bind");
    }

    //keep listening for data
    while (1)
    {
        printf("Waiting for data...\n");
        fflush(stdout);
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(sockfd, &seg, sizeof(udp_segment_t), 0, (struct sockaddr *)&si_other, &slen)) == -1)
        {
            die("recvfrom()");
        }

        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        // Compute the checksum to detect any error
        unsigned short sum = checksum(&seg);
        if (sum == seg.checksum)
        {
            output(&seg, "server.out");
        }
        else
        {
            printf("Error detected after checksum calculation. Discard it...\n");
        }

        memset(&seg, 0, sizeof(udp_segment_t));
    }

    close(sockfd);
    return 0;
}

