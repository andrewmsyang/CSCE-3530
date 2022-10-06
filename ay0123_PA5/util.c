/**
 * @file util.c
 * 
 * Define common function implementations used by both client and server sides.
 * 
 */
#include "util.h"

// Print the error and then exit the program
void die(char *s)
{
    perror(s);
    exit(1);
}

// Output segment information to both console and the given file
void prseg(dhcp_pkt *seg, char *filename)
{
    FILE *outfile = fopen(filename, "w");
    FILE *out[] = {outfile, stdout};
    for (int i = 0; i < 2; i++)
    {
        char ipaddr[32];
        inet_ntop(AF_INET, &seg->siaddr, ipaddr, 32);
        fprintf(out[i], "Sever IP: %s\n", ipaddr);
        inet_ntop(AF_INET, &seg->yiaddr, ipaddr, 32);
        fprintf(out[i], "Your IP: %s\n", ipaddr);
        fprintf(out[i], "Transaction ID: %d\n", seg->tran_ID);
        fprintf(out[i], "Lease time: %d\n\n", seg->lifetime);
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

void sendpkt(int sockfd, dhcp_pkt *seg, struct sockaddr_in *si)
{
    int slen = sizeof(struct sockaddr_in);
    sendto(sockfd, seg, sizeof(dhcp_pkt), 0, (struct sockaddr *)si, slen);
}

// Receive a segment from the given socket. The segment will be verified by
// computing the checksum value.
void receive(int sockfd, dhcp_pkt *seg, struct sockaddr_in *si)
{
    int slen = sizeof(struct sockaddr_in);
    recvfrom(sockfd, seg, sizeof(dhcp_pkt), 0, (struct sockaddr *)si, &slen);
}
