/* 
This is a USC-EE450 Project
Source code from: Socket Programming Reference - (Beej's-Guide)

** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "25831"  // the port users will be connecting to
#define UDP_PORT "24831"
#define MAXDATASIZE 100
#define BACKLOG 10	 // how many pending connections queue will hold

#define SERVER_A_PORT "21832"
#define SERVER_B_PORT "22831"
#define SERVER_C_PORT "23831"
void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


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

// TCP socket initialization
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;	// 3 address variable
						// hints is used for storing server address
						// *servinfo is used to store the available addresses for server
						// *p is used for loop in *servinfo
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;	// this variable indicates the size of socket
	struct sigaction sa;	// sigaction is a structure used to deal with zombie process
	int yes=1;	
	char s[INET6_ADDRSTRLEN];	// buffer used to store address of client (in form of xxx.xxx.xxx.xxx)
	int rv;			// used to store info of an particular host name, used for display only if error happens	
	int numbytes;
	float recv_buf[2];
	float operation;
	float operator;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {	// getaddrinfo function is load up a struct sock address for the third parameter, here, it is hints
									// parameter: servinfo is used to store the address list which you can choose from
									// return:    information on a particular host name (such as its IP address)
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {	// if no available address, exit
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}


	if (listen(sockfd, BACKLOG) == -1) {	// listen function is used to tell the socket to listen to connection request, as well as how many connections it can support
		perror("listen");		// socket is already created and binded in the loop above
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

// UDP server A initialization

	int udp_A_sockfd;
	struct addrinfo udp_A_hints, *udp_A_servinfo, *udp_A_p;
	int udp_A_rv;
	int udp_A_numbytes;

	memset(&udp_A_hints, 0, sizeof udp_A_hints);
	udp_A_hints.ai_family = AF_UNSPEC;
	udp_A_hints.ai_socktype = SOCK_DGRAM;

	if ((udp_A_rv = getaddrinfo("127.0.0.1", SERVER_A_PORT, &udp_A_hints, &udp_A_servinfo)) != 0) {	// server port is defined, the hostname should be 127.0.0.1
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_A_rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(udp_A_p = udp_A_servinfo; udp_A_p != NULL; udp_A_p = udp_A_p->ai_next) {
		if ((udp_A_sockfd = socket(udp_A_p->ai_family, udp_A_p->ai_socktype,
				udp_A_p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (udp_A_p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

// UDP server B initialization

	int udp_B_sockfd;
	struct addrinfo udp_B_hints, *udp_B_servinfo, *udp_B_p;
	int udp_B_rv;
	int udp_B_numbytes;

	memset(&udp_B_hints, 0, sizeof udp_B_hints);
	udp_B_hints.ai_family = AF_UNSPEC;
	udp_B_hints.ai_socktype = SOCK_DGRAM;

	if ((udp_B_rv = getaddrinfo("127.0.0.1", SERVER_B_PORT, &udp_B_hints, &udp_B_servinfo)) != 0) {	// server port is defined, the hostname should be 127.0.0.1
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_B_rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(udp_B_p = udp_B_servinfo; udp_B_p != NULL; udp_B_p = udp_B_p->ai_next) {
		if ((udp_B_sockfd = socket(udp_B_p->ai_family, udp_B_p->ai_socktype,
				udp_B_p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (udp_B_p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

// UDP server C initialization

	int udp_C_sockfd;
	struct addrinfo udp_C_hints, *udp_C_servinfo, *udp_C_p;
	int udp_C_rv;
	int udp_C_numbytes;

	memset(&udp_C_hints, 0, sizeof udp_C_hints);
	udp_C_hints.ai_family = AF_UNSPEC;
	udp_C_hints.ai_socktype = SOCK_DGRAM;

	if ((udp_C_rv = getaddrinfo("127.0.0.1", SERVER_C_PORT, &udp_C_hints, &udp_C_servinfo)) != 0) {	// server port is defined, the hostname should be 127.0.0.1
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_C_rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(udp_C_p = udp_C_servinfo; udp_C_p != NULL; udp_C_p = udp_C_p->ai_next) {
		if ((udp_C_sockfd = socket(udp_C_p->ai_family, udp_C_p->ai_socktype,
				udp_C_p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (udp_C_p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

// UDP listener initialization

	int udp_listener_sockfd;
	struct addrinfo udp_listener_hints, *udp_listener_servinfo, *udp_listener_p;
	int udp_listener_rv;
	int udp_listener_numbytes;
	struct sockaddr_storage udp_listener_their_addr;
	float udp_listener_buf;
	socklen_t udp_listener_addr_len;
	char udp_listener_s[INET6_ADDRSTRLEN];

	memset(&udp_listener_hints, 0, sizeof udp_listener_hints);
	udp_listener_hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	udp_listener_hints.ai_socktype = SOCK_DGRAM;
	udp_listener_hints.ai_flags = AI_PASSIVE; // use my IP

	if ((udp_listener_rv = getaddrinfo(NULL, UDP_PORT, &udp_listener_hints, &udp_listener_servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_listener_rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(udp_listener_p = udp_listener_servinfo; udp_listener_p != NULL; udp_listener_p = udp_listener_p->ai_next) {
		if ((udp_listener_sockfd = socket(udp_listener_p->ai_family, udp_listener_p->ai_socktype,
				udp_listener_p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(udp_listener_sockfd, udp_listener_p->ai_addr, udp_listener_p->ai_addrlen) == -1) {
			close(udp_listener_sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (udp_listener_p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(udp_listener_servinfo);

// main loop
	while(1) {  
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);	// accept function is only used in TCP server, create a child socket
											// parameter 1: mother socket
											// parameter 2: where to store connector's address
											// parameter 3: size of the connector's address
											// return: a new socket descriptor
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,					// convert IP address of 2nd parameter to human-readble IP address, stored in third parameter
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener	// here what is actually closed is the child's copy of the sockfd file descriptor
			if ((numbytes = recv(new_fd, recv_buf, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				exit(1);
			}

		operator = recv_buf[0];
		operation = recv_buf[1];

		printf ("the operator is %f\n", operator);
	
		if (recv_buf[1] == 0.0) {
			printf("the operation is DIV\n");
		} else {
			printf("the operation is LOG\n");
		}

		// instead of saying "hello", the aws works like a client, contact back-servers via UDP here

		//send message to server A
		if ((udp_A_numbytes = sendto(udp_A_sockfd, &operator, 4, 0,	// send to UDP server, the address is assigned in getaddrinfo function above
			 udp_A_p->ai_addr, udp_A_p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}

		freeaddrinfo(udp_A_servinfo);

		printf("AWS: sent %d bytes to server A\n", udp_A_numbytes);
		close(udp_A_sockfd);

		// receive from A

		printf("listener: waiting to recvfrom...\n");
		udp_listener_addr_len = sizeof udp_listener_their_addr;
		if ((udp_listener_numbytes = recvfrom(udp_listener_sockfd, &udp_listener_buf, 4 , 0,			//wait for the incoming packets
			(struct sockaddr *)&udp_listener_their_addr, &udp_listener_addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
	}	

		printf("listener: got packet from %s\n",				// translate into readable ip address, then print it out
			inet_ntop(udp_listener_their_addr.ss_family,
				get_in_addr((struct sockaddr *)&udp_listener_their_addr),
				udp_listener_s, sizeof udp_listener_s));
		printf("listener: packet is %d bytes long\n", udp_listener_numbytes);
		printf("listener: packet contains \"%f\"\n", udp_listener_buf);

		close(udp_listener_sockfd);
/*
		//send message to server B
		if ((udp_B_numbytes = sendto(udp_B_sockfd, &operator, 4, 0,	// send to UDP server, the address is assigned in getaddrinfo function above
			 udp_B_p->ai_addr, udp_B_p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}

		freeaddrinfo(udp_B_servinfo);

		printf("AWS: sent %d bytes to server B\n", udp_B_numbytes);
		close(udp_B_sockfd);

		//send message to server C
		if ((udp_C_numbytes = sendto(udp_C_sockfd, &operator, 4, 0,	// send to UDP server, the address is assigned in getaddrinfo function above
			 udp_C_p->ai_addr, udp_C_p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}

		freeaddrinfo(udp_C_servinfo);

		printf("AWS: sent %d bytes to server C\n", udp_C_numbytes);
		close(udp_C_sockfd);
*/
		if (send(new_fd, &operator, 4, 0) == -1)		// parameter 1: socket that's sending
									// parameter 2: pointer to what you want to send
									// parameter 3: size you send
									// parameter 4: flag
			perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}


