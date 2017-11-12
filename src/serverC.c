// Server C source code

/*
** This is a USC-EE450 Project
** Source code from: Socket Programming Reference - (Beej's-Guide)

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MYPORT "23831"	// the port AWS will be connecting to

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{

// recv socket
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	float buf;
	float send[2];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("The Server C is up and running using UDP on port "MYPORT"\n");

//main loop

while(1) {
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, &buf, 4 , 0,			//wait for the incoming packets
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("The Server C received input <%g>\n", buf);

	send[0] = buf * buf * buf * buf * buf;		// calculate x^2 here
	send[1] = 5.0;
	printf("The Server C calculated 5th power: <%g>\n", send[0]);

	if ((numbytes = sendto(sockfd, &send, 8, 0,	// send to UDP server, the address is assigned in getaddrinfo function above
			 (struct sockaddr *)&their_addr, addr_len)) == -1) {
		perror("senderr: sendto");
		exit(1);
	}

	printf("The Server C finished sending the output to AWS\n");
}
	return 0;
}
