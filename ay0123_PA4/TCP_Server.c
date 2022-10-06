/**
 * @file TCP_Server.c
 * 
 * This is the server part of my program demonstrating the handshaking and 
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

    struct sockaddr_in si_me, si_other;

    int sockfd, i, slen = sizeof(si_other), recv_len;
    tcp_segment_t rcvd, response;
    memset(&rcvd, 0, sizeof(tcp_segment_t));

    srand(1);
    unsigned int startseq = rand();

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

    // keep listening for data
    while (1)
    {
        // try to receive some data, this is a blocking call
        receive(sockfd, &rcvd, &si_other);

        if (getsyn(&rcvd))
        {
            // Receive the ACK packet
            populate(&response, rcvd.des, rcvd.src);

            response.seq = startseq;
            response.ack = rcvd.seq + 1;
            setsyn(&response);
            setack(&response);
            response.cksum = checksum(&response);

            prseg(&response, "server.out");

            // Send the SYNACK packet
            sendto(sockfd, &response, sizeof(tcp_segment_t), 0, (struct sockaddr *)&si_other, slen);

            // Receive the ACK
            receive(sockfd, &rcvd, &si_other);
            if (getack(&rcvd))
            {
                prmsg("Connection has been established\n\n", "server.out");
            }
        }

        if (getfin(&rcvd))
        {
            // Receive the FIN packet
            populate(&response, rcvd.des, rcvd.src);
            response.seq = 512;
            response.ack = rcvd.seq + 1;
            setack(&response);
            response.cksum = checksum(&response);
            prseg(&response, "server.out");
            // Send the ACK packet
            sendto(sockfd, &response, sizeof(tcp_segment_t), 0, (struct sockaddr *)&si_other, slen);

            populate(&response, rcvd.des, rcvd.src);
            response.seq = 512;
            response.ack = rcvd.seq + 1;
            setfin(&response);
            response.cksum = checksum(&response);
            prseg(&response, "server.out");
            // Send the FIN packet
            sendto(sockfd, &response, sizeof(tcp_segment_t), 0, (struct sockaddr *)&si_other, slen);

            // Receive the ACK packet
            receive(sockfd, &rcvd, &si_other);
            if (getack(&rcvd))
            {
                prmsg("Connection is closed\n\n", "server.out");
                exit(0);
            }
        }
    }

    close(sockfd);
    return 0;
}

