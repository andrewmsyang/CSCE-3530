/**
 * @file DHCP_Client.c
 * 
 * This is the client part of my program demonstrating the handshaking and 
 * termination of a TCP connection. To simulate a concrete process, the inner 
 * implementation still utilizes the UDP socket instead of a TCP socket.
 * 
 */
#include "util.h"
#include <time.h>

int main(int argc, char **argv)
{
    if (argc < 2)
        return 0;

    // Get the destination port
    int dstport = atoi(argv[1]);

    struct sockaddr_in si_source, si_other;
    int sockfd, i = 0, slen = sizeof(si_other);

    // Create a TCP segment
    dhcp_pkt seg, rcvd;

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

    // Demonstrate the DHCP requesting process

    // Send the discover packet
    int tranid = rand();
    inet_pton(AF_INET, SERVER, &seg.siaddr);
    seg.yiaddr = 0;
    seg.tran_ID = tranid;
    prseg(&seg, "client.out");
    sendpkt(sockfd, &seg, &si_other);

    // Receive the offer packet and respond with request packet
    receive(sockfd, &rcvd, &si_other);
    seg = rcvd;
    seg.tran_ID = tranid + 1;
    prseg(&seg, "client.out");
    sendpkt(sockfd, &seg, &si_other);

    while (1)
        ;

    return 0;
}

