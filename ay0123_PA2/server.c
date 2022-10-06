#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define LOGGING_FNAME "list.txt"

char http_resp_buff[10000000];

int get_cache_fname(char* url, char* cache_hit_fname, int *page_cached) {
    /* get the cache file name from the logging file if the webpage is cached. */
    FILE *fp;
    char line_buff[1024];
    char cache_url[1024];
    char cache_fname[32];
    const int MAX_LINE_NO = 5;  // #cached_url to check at most

    // open the logging file
    if ((fp = fopen(LOGGING_FNAME,"r")) == NULL) {
        perror("Fail to open the cache file!\n");
        exit(1);
    }

    // only read several lines of the logging file (recent cache)
    int line_no = 0;
    for (; line_no < MAX_LINE_NO; ++line_no) {
        if (fgets(line_buff, sizeof(line_buff), fp) == NULL) break;   // read a new line
        sscanf(line_buff, "%[^ ]", cache_url);          // parse the url
        sscanf(line_buff, "%*s %s", cache_fname);       // parse the filename

        // if the cached url and the client request match, set the flag `page_cached` to be 1 and store the filename
        if (strcmp(cache_url, url) == 0) {
            *page_cached = 1;
            strcpy(cache_hit_fname, cache_fname);
        }
    }

    // close the logging file
    fclose(fp);
    return 0;
}

int host_name2ip(char *host_name, char *ip) {
    struct hostent *hp;
    if ((hp = gethostbyname(host_name)) == NULL) {
        perror("Fail to get host by name.\n");
        exit(1);
    }
    strcpy(ip, inet_ntoa(*(struct in_addr *) hp -> h_addr_list[0]));
    return 0;
}

int main(int argc, char* argv[]){
    // initialize the server socket
	int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sockfd < 0) {
		perror("Fail to create the server socket.\n");
		exit(1);
	}

	// build sockaddr of the server
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;    
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_addr.sin_port = htons(strtol(argv[1], NULL, 10)); 

	// bind the socket
	if (bind(server_sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("Fail to bind the socket.\n");
		exit(1);
	}

	// set the socket listening
	if (listen(server_sockfd, 10) < 0 ){
		perror("Fail to listen\n");
		exit(1);
	}

	// receive the client request
    int conn_fd = accept(server_sockfd, (struct sockaddr*)NULL, NULL);
    char client_recv_buff[1024];
	read(conn_fd, client_recv_buff, sizeof(client_recv_buff));
	printf("Receive client request url: %s\n", client_recv_buff);

	// check whether the request_url is cached recently
    int page_cached = 0;
    char cache_fname[32] = {0};
    get_cache_fname(client_recv_buff, cache_fname, &page_cached);

    FILE *fp;
    char cache_line[8192];
    // if cached webpage is found, the cached file is sent to the client
    if (page_cached == 1 && (fp = fopen(cache_fname,"r")) != NULL) {
        printf("Cache file %s found for webpage %s\n", cache_fname, client_recv_buff);
        while ((fgets(cache_line, sizeof(cache_line), fp)) != NULL){
            strcat(http_resp_buff, cache_line);
        }
        send(conn_fd, http_resp_buff, strlen(http_resp_buff), 0);
    }
    else {  // url not cached, send HTTP request and forward the response
        // construct HTTP request
        char http_req[2048];
        char host_name[512];
        char host_path[512];
        sscanf(client_recv_buff,"%[^/\n]", host_name);  // parse host name from client request
        sscanf(client_recv_buff,"%*[^/]/%[^\n]", host_path);  // parse relative path
        sprintf(http_req, "GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/html\r\n\r\n", host_path, host_name);

        // get host IP string
        char ip[20] = {0};
        host_name2ip(host_name, ip);
        printf("Host Ip: %s\n", ip);

        // build socket address of the HTTP server
        struct sockaddr_in http_addr;
        http_addr.sin_addr.s_addr = inet_addr(ip);
        http_addr.sin_family = AF_INET;
        http_addr.sin_port = htons(80);

        // initialize the http socket
        int http_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (http_sockfd < 0) {
            perror("Fail to create the http socket.\n");
            exit(1);
        }

        // connect to the http server
        if (connect(http_sockfd, (struct sockaddr *)&http_addr, sizeof(http_addr)) < 0) {
            perror("Fail to connect to the http server");
            exit(1);
        }
        printf("Connected to the HTTP server, waiting for the response...\n");

        // send http request to the http server, receive the response
        send(http_sockfd, http_req, strlen(http_req), 0);
        bzero(http_resp_buff, sizeof(http_resp_buff));
        recv(http_sockfd, http_resp_buff, sizeof(http_resp_buff), MSG_WAITALL);

        // close the HTTP socket
        close(http_sockfd);

        // parse the status code from the HTTP response
        int status_code = -1;
        char *buff_p = strstr(http_resp_buff, "HTTP/");
        if (buff_p) {
            sscanf(buff_p, "%*s %d", &status_code);
        }
        printf("Status code: %d\n", status_code);

        // if the status code is 200, cache the file and forward the page
        if (status_code == 200) {
            // get the cache filename using the timestamp
            time_t timer = time(NULL);
            strftime(cache_fname, 32, "%Y%m%d%H%M%S", localtime(&timer));

            // move the pointer to the message body
            buff_p = strstr(http_resp_buff, "<!DOCTYPE html>");

            // store the webpage into the new cache file
            if ((fp = fopen(cache_fname, "w")) != NULL) {
                fputs(buff_p, fp);
                fclose(fp);
                // if the webpage is cached successfully, update the logging file by adding a new line at the top of it
                char logging_buff[1000000] = {0};
                char line_buff[1024] = {0};
                if ((fp = fopen(LOGGING_FNAME, "r")) != NULL) {
                    sprintf(logging_buff, "%s %s\n", client_recv_buff, cache_fname);
                    while (fgets(line_buff, sizeof(line_buff), fp) != NULL){
                        strcat(logging_buff, line_buff);
                    }
                    fclose(fp);
                }

                if ((fp = fopen(LOGGING_FNAME, "w")) != NULL) {
                    fputs(logging_buff, fp);
                    fclose(fp);
                }
            }

            // send the mesage to the client
            send(conn_fd, buff_p, strlen(buff_p), 0);
        }
        else {  // forward the response directly if the response status code is not 200
            send(conn_fd, http_resp_buff, strlen(http_resp_buff), 0);
        }

    }

    // close the server socket
	close(server_sockfd);
	return 0;
}

