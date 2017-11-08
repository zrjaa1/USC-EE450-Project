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

#define PORT "3490"  // the port users will be connecting to
#define MAXDATASIZE 100
#define BACKLOG 10	 // how many pending connections queue will hold

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
	char buf[MAXDATASIZE];
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

	while(1) {  // main accept() loop
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
			if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				exit(1);
			} 
	// instead of saying "hello", the aws works like a client, contact back-servers via UDP here

			if (send(new_fd, &buf, numbytes, 0) == -1)	// parameter 1: socket that's sending
										// parameter 2: what you want to send
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


