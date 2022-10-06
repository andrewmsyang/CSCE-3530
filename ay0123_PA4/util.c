/**
 * @file util.c
 * 
 * Define common function implementations used by both client and server sides.
 * 
 * 
 */
#include "util.h"

// Calculate the checksum for a given segment
unsigned short checksum(tcp_segment_t *pseg)
{
    tcp_segment_t seg = *pseg;
    seg.cksum = 0;
    unsigned short *cksum_arr = (unsigned short *)&seg;
    unsigned int sum = 0;
    for (int i = 0; i < 524; i++) // Compute sum
    {
        sum = sum + cksum_arr[i];
    }

    int wrap = sum >> 16; // Wrap around
    sum = sum & 0x0000FFFF;
    sum = wrap + sum;

    // Wrap around once more as the previous sum could have generated a carry
    wrap = sum >> 16;
    sum = sum & 0x0000FFFF;
    unsigned int cksum = wrap + sum;

    /* XOR the sum for checksum */
    return 0xFFFF ^ cksum;
}

// Print the error and then exit the program
void die(char *s)
{
    perror(s);
    exit(1);
}

// Set the SYN flag to the given segment
void setsyn(tcp_segment_t *seg)
{
    seg->hdr_flags = (seg->hdr_flags | SYNMASK);
}

// Get the SYN flag of the given segment
int getsyn(tcp_segment_t *seg)
{
    if ((seg->hdr_flags & SYNMASK) == SYNMASK)
        return 1;
    return 0;
}

// Set the ACK flag to the given segment
void setack(tcp_segment_t *seg)
{
    seg->hdr_flags = (seg->hdr_flags | ACKMASK);
}

// Get the ACK flag of the given segment
int getack(tcp_segment_t *seg)
{
    if ((seg->hdr_flags & ACKMASK) == ACKMASK)
        return 1;
    return 0;
}

// Set the FIN flag to the given segment
void setfin(tcp_segment_t *seg)
{
    seg->hdr_flags = (seg->hdr_flags | FINMASK);
}

// Get the FIN flag of the segment
int getfin(tcp_segment_t *seg)
{
    if ((seg->hdr_flags & FINMASK) == FINMASK)
        return 1;
    return 0;
}

// Output segment information to both console and the given file
void prseg(tcp_segment_t *seg, char *filename)
{
    FILE *outfile = fopen(filename, "w");
    FILE *out[] = {outfile, stdout};
    for (int i = 0; i < 2; i++)
    {
        fprintf(out[i], "Source port: %d\n", seg->src);
        fprintf(out[i], "Destination port: %d\n", seg->des);
        fprintf(out[i], "SEQ: %d\n", seg->seq);
        fprintf(out[i], "ACK: %d\n", seg->ack);
        fprintf(out[i], "Header length: %d\n", seg->hdr_flags >> 12);
        fprintf(out[i], "SYN flag: %d\n", getsyn(seg));
        fprintf(out[i], "ACK flag: %d\n", getack(seg));
        fprintf(out[i], "FIN flag: %d\n", getfin(seg));
        fprintf(out[i], "Checksum: %d\n\n", seg->cksum);
    }
    fclose(outfile);
}

// Output a text message to both console and the given file
void prmsg(const char *msg, char *filename)
{
    FILE *outfile = fopen(filename, "w");
    fprintf(stdout, "%s", msg);
    fprintf(outfile, "%s", msg);
    fclose(outfile);
}

// Empty the given segment and then populate the source, destination and offset
void populate(tcp_segment_t *seg, unsigned int src, unsigned int des)
{
    bzero(seg, 1048);
    seg->src = src;
    seg->des = des;
    seg->hdr_flags = (6 << 12);
}

// Receive a segment from the given socket. The segment will be verified by
// computing the checksum value.
void receive(int sockfd, tcp_segment_t *seg, struct sockaddr_in *si)
{
    int slen = sizeof(struct sockaddr_in);
    recvfrom(sockfd, seg, sizeof(tcp_segment_t), 0, (struct sockaddr *)si, &slen);
    unsigned short cksum = checksum(seg);
    if (cksum != seg->cksum)
        exit(1);
}
