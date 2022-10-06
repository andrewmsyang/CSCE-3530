/*
    Simple udp client
*/
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Run client and server on the same local machine when using the following IP
// #define SERVER "127.0.0.1"
#define SERVER "129.120.151.96"
#define BUFLEN 1024 // Max length of payload buffer

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
    if (argc < 3)
        return 0;

    // Get the destination port
    int dstport = atoi(argv[1]);

    struct sockaddr_in si_source, si_other;
    int sockfd, i = 0, slen = sizeof(si_other);
    // Create a UDP segment
    udp_segment_t seg;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    memset((char *)&si_source, 0, sizeof(si_source));
    si_source.sin_family = AF_INET;
    si_source.sin_addr.s_addr = htonl(INADDR_ANY);
    si_source.sin_port = 0;
    // Bind sockfd to si_source to retrieve the source port later
    if (bind(sockfd, (struct sockaddr *)&si_source, slen) == -1)
    {
        die("bind");
    }

    // Get the address to which the sockfd is bound
    getsockname(sockfd, (struct sockaddr *)&si_source, &slen);
    int srcport = ntohs(si_source.sin_port);

    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(dstport);
    if (inet_aton(SERVER, &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    // Populate source port and destination port to the segment
    seg.srcport = srcport;
    seg.dstport = dstport;

    // Populate length and payload fields by the given input file
    FILE *f = fopen(argv[2], "r");
    fseek(f, 0, SEEK_END);
    seg.length = ftell(f);
    rewind(f);
    if (seg.length >= BUFLEN)
        seg.length = BUFLEN - 1;
    fread(seg.payload, seg.length, 1, f);
    seg.payload[seg.length] = '\0';
    fclose(f);

    // Compute the checksum and populate its associated field
    seg.checksum = checksum(&seg);

    // Send the message
    if (sendto(sockfd, &seg, sizeof(udp_segment_t), 0, (struct sockaddr *)&si_other, slen) == -1)
    {
        die("sendto()");
    }

    // Output to console and the client.out file
    output(&seg, "client.out");

    close(sockfd);
    return 0;
}

