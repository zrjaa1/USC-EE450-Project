// Server A source code

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

#define MYPORT "21832"	// the port AWS will be connecting to
#define AWSPORT "24831" // the port of AWS
#define MAXBUFLEN 100

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
	float send;
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

	printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, &buf, 4 , 0,			//wait for the incoming packets
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	

	send = buf * buf;		// calculate x^2 here

	printf("listener: got packet from %s\n",				// translate into readable ip address, then print it out
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);
	printf("listener: packet contains \"%f\"\n", buf);

	close(sockfd);

// assign a new socket to send message to AWS
	int send_sockfd;
	struct addrinfo send_hints, *send_servinfo, *send_p;
	int send_rv;
	int send_numbytes;
	
	memset(&send_hints, 0, sizeof send_hints);
	send_hints.ai_family = AF_UNSPEC;
	send_hints.ai_socktype = SOCK_DGRAM;

	if ((send_rv = getaddrinfo("127.0.0.1", AWSPORT, &send_hints, &send_servinfo)) != 0) {	// server port is defined, the hostname should be 127.0.0.1
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(send_rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(send_p = send_servinfo; send_p != NULL; send_p = send_p->ai_next) {
		if ((send_sockfd = socket(send_p->ai_family, send_p->ai_socktype,
				send_p->ai_protocol)) == -1) {
			perror("sender: socket");
			continue;
		}

		break;
	}

	if (send_p == NULL) {
		fprintf(stderr, "sender: failed to create socket\n");
		return 2;
	}

	if ((send_numbytes = sendto(send_sockfd, &send, 4, 0,	// send to UDP server, the address is assigned in getaddrinfo function above
			 send_p->ai_addr, send_p->ai_addrlen)) == -1) {
		perror("senderr: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);

	printf("Server A: sent %d bytes to AWS\n", send_numbytes);
	close(send_sockfd);

	return 0;
}
