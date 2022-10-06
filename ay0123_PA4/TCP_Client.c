/**
 * @file TCP_Client.c
 * 
 * This is the client part of my program demonstrating the handshaking and 
 * termination of a TCP connection. To simulate a concrete process, the inner 
 * implementation still utilizes the UDP socket instead of a TCP socket.
 * 
 * 
 */
#include "util.h"

int main(int argc, char **argv)
{
    if (argc < 2)
        return 0;

    // Get the destination port
    int dstport = atoi(argv[1]);

    struct sockaddr_in si_source, si_other;
    int sockfd, i = 0, slen = sizeof(si_other);

    srand(2);
    unsigned int startseq = rand();

    // Create a TCP segment
    tcp_segment_t seg, rcvd;

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

    // Demonstrate the handshaking process

    populate(&seg, srcport, dstport);
    seg.seq = startseq;
    setsyn(&seg);
    seg.cksum = checksum(&seg);

    prseg(&seg, "client.out");

    // Send the SYN message
    sendto(sockfd, &seg, sizeof(tcp_segment_t), 0, (struct sockaddr *)&si_other, slen);

    // Receive the SYNACK message
    receive(sockfd, &rcvd, &si_other);
    if (getack(&rcvd) && getsyn(&rcvd))
    {
        populate(&seg, srcport, dstport);
        seg.seq = rcvd.ack;
        seg.ack = rcvd.seq + 1;
        setack(&seg);
        seg.cksum = checksum(&seg);

        prseg(&seg, "client.out");

        // Send the ACK message
        sendto(sockfd, &seg, sizeof(tcp_segment_t), 0, (struct sockaddr *)&si_other, slen);
    }

    // Demonstrate the closing process
    populate(&seg, srcport, dstport);
    seg.seq = 1024;
    seg.ack = 512;
    setfin(&seg);
    seg.cksum = checksum(&seg);
    prseg(&seg, "client.out");
    // Send the FIN packet
    sendto(sockfd, &seg, sizeof(tcp_segment_t), 0, (struct sockaddr *)&si_other, slen);

    receive(sockfd, &rcvd, &si_other);
    if (getack(&rcvd))
    {
        receive(sockfd, &rcvd, &si_other);
        if (getfin(&rcvd))
        {
            populate(&seg, srcport, dstport);
            seg.seq = 1025;
            seg.ack = rcvd.seq + 1;
            setack(&seg);
            seg.cksum = checksum(&seg);
            prseg(&seg, "client.out");
            // Send the last ACK packet
            sendto(sockfd, &seg, sizeof(tcp_segment_t), 0, (struct sockaddr *)&si_other, slen);
        }
    }
    return 0;
}

