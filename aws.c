
/* AWS source code: 1 TCP socket + 1 UDP socket

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
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "25831"  // the port users will be connecting to
#define UDP_PORT "24831"
#define MAXDATASIZE 100
#define BACKLOG 10	 // how many pending connections queue will hold

#define SERVER_A_PORT "21831"
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

	// TCP socket initialization: (from Beej's-Guide: server.c)
	int sockfd, new_fd;  			// listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;	// 3 address variable
						// hints is used for storing server address
						// *servinfo is used to store the available addresses for server
						// *p is used for loop in *servinfo
	struct sockaddr_storage their_addr; 	// connector's address information
	socklen_t sin_size;			// this variable indicates the size of socket
	struct sigaction sa;			// sigaction is a structure used to deal with zombie process
	int yes=1;	
	int i;
	int port_number;
	char s[INET6_ADDRSTRLEN];		// buffer used to store address of client (in form of xxx.xxx.xxx.xxx)
	int rv;					// used to store info of an particular host name, used for display only if error happens	
	int numbytes;				// used to indicate the number of data being sent/received, in bytes
	float recv_buf[2];		
	float operation;			// 0.0 = DIV, 1.0 = LOG
	float operator;				// the value of x received from client
	float x2;				// x^2, x^3, x^4, x^5, x^6
	float x3;	
	float x4;
	float x5;
	float x6;
	float result;				// the final result that will be sent to client

	
	// initialize the socket parameter
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; 		// use my IP

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

	freeaddrinfo(servinfo); 		// all done with this structure

	if (p == NULL)  {			// if no available address, exit
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}


	if (listen(sockfd, BACKLOG) == -1) {	// listen function is used to tell the socket to listen to connection request, as well as how many connections it can support
		perror("listen");		// socket is already created and binded in the loop above
		exit(1);
	}

	sa.sa_handler = sigchld_handler;	// reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	// UDP socket initialization: (from Beej's-Guide: listener.c)

	int udp_sockfd;
	struct addrinfo udp_hints, *udp_servinfo, *udp_p;
	int udp_rv;
	struct sockaddr_storage udp_their_addr;
	float udp_buf[2];
	socklen_t udp_addr_len;
	char udp_s[INET6_ADDRSTRLEN];

	memset(&udp_hints, 0, sizeof udp_hints);
	udp_hints.ai_family = AF_UNSPEC; 	// set to AF_INET to force IPv4
	udp_hints.ai_socktype = SOCK_DGRAM;
	udp_hints.ai_flags = AI_PASSIVE;	 // use my IP

	if ((udp_rv = getaddrinfo(NULL, UDP_PORT, &udp_hints, &udp_servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(udp_p = udp_servinfo; udp_p != NULL; udp_p = udp_p->ai_next) {
		if ((udp_sockfd = socket(udp_p->ai_family, udp_p->ai_socktype,
				udp_p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(udp_sockfd, udp_p->ai_addr, udp_p->ai_addrlen) == -1) {
			close(udp_sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (udp_p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(udp_servinfo);

	// UDP server A's addr initialization (from Beej's Guide: talker.c)

	struct addrinfo udp_A_hints, *udp_A_servinfo, *udp_A_p;
	int udp_A_rv;

	memset(&udp_A_hints, 0, sizeof udp_A_hints);
	udp_A_hints.ai_family = AF_UNSPEC;
	udp_A_hints.ai_socktype = SOCK_DGRAM;

	if ((udp_A_rv = getaddrinfo("127.0.0.1", SERVER_A_PORT, &udp_A_hints, &udp_A_servinfo)) != 0) {	// server port is defined, the hostname should be 127.0.0.1
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_A_rv));
		return 1;
	}

	udp_A_p = udp_A_servinfo; // the udp_A_p is what we actually need in the future to set where the UDP datagram will be sent to.

	// UDP server B's addr initialization (from Beej's Guide: talker.c)

	struct addrinfo udp_B_hints, *udp_B_servinfo, *udp_B_p;
	int udp_B_rv;

	memset(&udp_B_hints, 0, sizeof udp_B_hints);
	udp_B_hints.ai_family = AF_UNSPEC;
	udp_B_hints.ai_socktype = SOCK_DGRAM;

	if ((udp_B_rv = getaddrinfo("127.0.0.1", SERVER_B_PORT, &udp_B_hints, &udp_B_servinfo)) != 0) {	// server port is defined, the hostname should be 127.0.0.1
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_B_rv));
		return 1;
	}

	udp_B_p = udp_B_servinfo; // the same as above

	// UDP server C's addr initialization (from Beej's Guide: talker.c)

	struct addrinfo udp_C_hints, *udp_C_servinfo, *udp_C_p;
	int udp_C_rv;

	memset(&udp_C_hints, 0, sizeof udp_C_hints);
	udp_C_hints.ai_family = AF_UNSPEC;
	udp_C_hints.ai_socktype = SOCK_DGRAM;

	if ((udp_C_rv = getaddrinfo("127.0.0.1", SERVER_C_PORT, &udp_C_hints, &udp_C_servinfo)) != 0) {	// server port is defined, the hostname should be 127.0.0.1
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_C_rv));
		return 1;
	}

	udp_C_p = udp_C_servinfo;

	printf("The AWS is up and running.\n"); // AWS initialization is finished, and wait for connection from client.

	// main loop:
	// receive operator and operation from client via TCP, and send the operator send backend-servers, then receive result from backend-servers and calculate the final result.
	// after calculate the final result, send the result back to client

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

		inet_ntop(their_addr.ss_family,		// convert IP address of 2nd parameter to human-readble IP address, stored in third parameter
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);

		if (!fork()) { 				// this is the child process
			close(sockfd); 			// here what is actually closed is the child's copy of the sockfd file descriptor

			//receive operation and operator from client's input
			if ((numbytes = recv(new_fd, recv_buf, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				exit(1);
			}

			operator = recv_buf[0];
			operation = recv_buf[1];

			printf("The AWS received <%g>", operator);
			if (operation == 0.0)
				printf(" and function=DIV from the client using TCP over port "PORT"\n");
			else
				printf(" and function=LOG from the client using TCP over port "PORT"\n");

			//send x to server A
			if ((numbytes = sendto(udp_sockfd, &operator, 4, 0,	// send to UDP server, the address is assigned in getaddrinfo function above
				 udp_A_p->ai_addr, udp_A_p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}

			printf("The AWS sent <%g> to Backend-Server A\n", operator);

			//send x to server B
			if ((numbytes = sendto(udp_sockfd, &operator, 4, 0,	// send to UDP server, the address is assigned in getaddrinfo function above
				 udp_B_p->ai_addr, udp_B_p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}

			printf("The AWS sent <%g> to Backend-Server B\n", operator);

			//send x to server C
			if ((numbytes = sendto(udp_sockfd, &operator, 4, 0,	// send to UDP server, the address is assigned in getaddrinfo function above
				 udp_C_p->ai_addr, udp_C_p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}

			freeaddrinfo(udp_C_servinfo);

			printf("The AWS sent <%g> to Backend-Server C\n", operator);

			// receive x^2, x^3, x^5 from A, B, C
		
			for (i=0; i<=2; i++) {
				udp_addr_len = sizeof udp_their_addr;
				if ((numbytes = recvfrom(udp_sockfd, &udp_buf, 8 , 0,			//wait for the incoming packets
					(struct sockaddr *)&udp_their_addr, &udp_addr_len)) == -1) {
					perror("recvfrom");
					exit(1);
				}	
	
				if (udp_buf[1] == 2.0) {	
					x2 = udp_buf[0];
					printf("The AWS received <%g> from Backend-Server A using UDP over port <"SERVER_A_PORT">\n", udp_buf[0]);
				} else if (udp_buf[1] == 3.0) {
					x3 = udp_buf[0];
					printf("The AWS received <%g> from Backend-Server B using UDP over port <"SERVER_B_PORT">\n", udp_buf[0]);
				} else if (udp_buf[1] == 5.0) {
					x5 = udp_buf[0];
					printf("The AWS received <%g> from Backend-Server C using UDP over port <"SERVER_C_PORT">\n", udp_buf[0]);
				} else 
					printf("Error, received unkown message\n");
			}

			//send x^2 to server A
			if ((numbytes = sendto(udp_sockfd, &x2, 4, 0,	// send to UDP server, the address is assigned in getaddrinfo function above
				 udp_A_p->ai_addr, udp_A_p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}

			freeaddrinfo(udp_A_servinfo);

			printf("The AWS sent <%g> to Backend-Server A\n", x2);

			//send x^2 to server B
			if ((numbytes = sendto(udp_sockfd, &x2, 4, 0,	// send to UDP server, the address is assigned in getaddrinfo function above
				 udp_B_p->ai_addr, udp_B_p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}

			freeaddrinfo(udp_B_servinfo);

			printf("The AWS sent <%g> to Backend-Server B\n", x2);

			// receive x^4, x^6 from A, B
		
			for (i=0; i<=1; i++) {
				udp_addr_len = sizeof udp_their_addr;
				if ((numbytes = recvfrom(udp_sockfd, &udp_buf, 8 , 0,			//wait for the incoming packets
					(struct sockaddr *)&udp_their_addr, &udp_addr_len)) == -1) {
					perror("recvfrom");
					exit(1);
				}	

				if (udp_buf[1] == 2.0) {	
					x4 = udp_buf[0];
					printf("The AWS received <%g> from Backend-Server A using UDP over port <"SERVER_A_PORT">\n", udp_buf[0]);
				} else if (udp_buf[1] == 3.0) {
					x6 = udp_buf[0];
					printf("The AWS received <%g> from Backend-Server B using UDP over port <"SERVER_B_PORT">\n", udp_buf[0]);
				} else 
					printf("Error, received unkown message\n");
			}
		
			close(udp_sockfd);

			printf("Values of powers received by AWS:<%g,%g,%g,%g,%g,%g>\n", operator, x2, x3, x4, x5, x6);

			// now that we get all the results from backend-server, send the result to client.

			if (operation == 0.0) {
				result = 1 + operator + x2 + x3 + x4 + x5 + x6;
				printf("AWS calculated DIV on <%g>:<%g>\n", operator, result);
			}  else if (operation == 1.0) {
				result = -operator - x2/2 - x3/3 - x4/4 - x5/5 - x6/6; 	
				printf("AWS calculated LOG on <%g>:<%g>\n", operator, result);
			}  else {
				printf("Error, unknown operation type\n");
				return 0;
			}

			if (send(new_fd, &result, 4, 0) == -1)			// parameter 1: socket that's sending
										// parameter 2: pointer to what you want to send
										// parameter 3: size you send
										// parameter 4: flag
				perror("send");
			else	
				printf("The AWS send <%f> to client\n", result);// if send successes, print if out			

				close(new_fd);
				exit(0);
		} 
               
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}


