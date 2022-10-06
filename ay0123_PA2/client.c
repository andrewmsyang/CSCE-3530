#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

char recv_buff[10000000];

int main(int argc, char* argv[]){
	// initialize the client socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);  
	if (sockfd < 0) {
		perror("Fail to create the client socket.\n");
		exit(1);
	}
	
	// build sock_addr of the server
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("129.120.151.94");
	server_addr.sin_port = htons(strtol(argv[1], NULL, 10));

	// connect to the server
	int conn_fd = connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr));
	if (conn_fd < 0) {
		perror("Fail to connect to the server.\n");
		exit(1);
	}

	// get input from stdin
	char url[1024];
	printf("url: ");
	scanf("%s", url);

	// send url request to the sever
	write(sockfd, url, strlen(url));
	
	// receive the response from the server
	recv(sockfd, recv_buff, sizeof(recv_buff), MSG_WAITALL);
	printf("%s\n", recv_buff);

	// close the sockets
	close(sockfd);
	return 0;
}
