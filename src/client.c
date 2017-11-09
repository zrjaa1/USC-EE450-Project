/*
** Source code from: Socket Programming Reference - (Beej's-Guide)
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "25831" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	float send_buf[2];		// send_buf[0] = operator, send_buf[1] = operation			
	float* recv_buf;			// receive a float from server as the result
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc == 3 && strcmp(argv[0], "./client") == 0 && (strcmp(argv[1], "DIV") == 0 || strcmp(argv[1], "LOG") == 0));
	else {	
	    fprintf(stderr,"usage: client LOG/DIV x\n");
	    exit(1);
	}

	send_buf[0] = atof(argv[2]);

	if (strcmp(argv[1], "DIV") == 0) 
		send_buf[1] = 0.0;
	else
		send_buf[1] = 1.0;

	printf("the 0th in the send_buf is %f\n", send_buf[0]);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo("127.0.0.1", PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	if (send(sockfd, &send_buf, 8, 0) == -1)	// send the value of x to aws
		 perror("send");

	if ((numbytes = recv(sockfd, recv_buf, MAXDATASIZE-1, 0)) == -1) {	// numbytes: the number of data you actually received
									// recv_buf: where to store	
	    perror("recv");
	    exit(1);
	}

	printf("client: received result: %f\n", *recv_buf);

	close(sockfd);

	return 0;
}

