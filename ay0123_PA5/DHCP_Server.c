/**
 * @file DHCP_Server.c
 * 
 * This is the server part of my program demonstrating the handshaking and 
 * termination of a TCP connection. To simulate a concrete process, the inner 
 * implementation still utilizes the UDP socket instead of a TCP socket.
 * 
 */
#include "util.h"
#include <math.h>

int nextslot(int pool[], int maxnum)
{
    for (int i = 1; i < maxnum - 1; i++)
    {
        if (!pool[i])
        {
            pool[i] = 1;
            return i;
        }
    }
    return -1;
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return 0;

    struct sockaddr_in si_me, si_other;

    int sockfd, i, slen = sizeof(si_other), recv_len;
    dhcp_pkt rcvd, response;
    memset(&rcvd, 0, sizeof(dhcp_pkt));

    // create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    // zero out the structure
    memset((char *)&si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(atoi(argv[1]));
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind socket to port
    if (bind(sockfd, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
    {
        die("bind");
    }

    // prompt for network address and subnet mask
    char ipaddr[32];
    int subnet = 0;
    printf("network address: ");
    scanf("%s", ipaddr);
    unsigned int netaddr;
    inet_pton(AF_INET, ipaddr, &netaddr);
    netaddr = ntohl(netaddr);
    printf("subnet_part: ");
    scanf("%d", &subnet);
    int maxnum = pow(2, 32 - subnet);
    int pool[maxnum];
    memset(pool, 0, maxnum * sizeof(int));

    printf("\n");
    // keep listening for data
    while (1)
    {
        // try to receive some data, this is a blocking call
        receive(sockfd, &rcvd, &si_other);
        if (rcvd.yiaddr == 0)
        {
            // Send the offer packet
            int next = netaddr + nextslot(pool, maxnum);
            response.siaddr = rcvd.siaddr;
            response.yiaddr = htonl(next);
            response.tran_ID = rcvd.tran_ID;
            response.lifetime = 3600;
        }
        else
        {
            // Send the ack packet
            response = rcvd;
        }
        prseg(&response, "server.out");
        sendto(sockfd, &response, sizeof(dhcp_pkt), 0, (struct sockaddr *)&si_other, slen);
    }

    close(sockfd);
    return 0;
}

