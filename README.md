# USC-EE450-Project
This is a project source code for USC "EE450 Computer network". 

In this project, we are supposed to create a Client-Server program, consists of a central server called AWS, and 3 distributed servers called Backend-Server A, B and C. The client will connect with AWS, via TCP, sending the operation DIV or LOG, as well as a value x. The AWS will give the answer back to client, which is calculated by Taylor's Expansion: (note |x| < 1 here)
	1/(1-x) = 1 + x + x^2 + x^3 + x^4 + x^5 + x^6;
	log(1-x) = -x - x^2/2 - x^3/3 - x^4/4 - x^5/5 - x^6/6;

## a. What I have done in the assignment?
	1. Built TCP and UDP connection between clients and servers. 
	2. Well-defined data format and accomplished the TCP and UDP message exchange.

## b. What my code files are and what each one of them does?
	aws.c:		1 UDP socket and 1 TCP socket is created in this file. The TCP socket is responsible for connecting with client, get the input and send the result back to client. The UDP socket is 
			responsible for connecting with backend-servers to get value of x^2, x^3, x^4, x^5 and x^6.
	client.c:	1 TCP socket is created in this file. The client get x and LOG/DIV from user's inputs and send it to AWS, then wait for answer from AWS.
	serverA.c:	1 UDP socket is created in this file. The backend-serverA get value of x and send x^2 to AWS. 
	serverB.c:      1 UDP socket is created in this file. The backend-serverA get value of x and send x^3 to AWS.
	serverC.c:	1 UDP socket is created in this file. The backend-serverA get value of x and send x^5 to AWS.

## c. The format of all the message exchanged
	User's input: 	./client DIV/LOG x, these parameters will be stored in a string array called "argv[]". 
	In client.c:  	"float send_buf[2]" buf[0] = value of x, buf[1] = 0.0 (means DIV) or 1.0 (means LOG).
	In aws.c:       "float recv_buf[2]" to store data from client; 
			"float operator"    to store value of x and is sent to backend-servers
			"float x2,x3,x4,x5,x6"   to store value of x^2, x^3, x^4, x^5, x^6 received from backend-servers
			"float result" 	    to store the final value and is sent to client
	In serverA.c:   "float operator"    to store the value of x received from AWS
			"float result" 	    to store the power of x and is sent to AWS
	In serverB.c  	similar to serverA.c
	In serverC.c 	similar to serverA.c

## d. Idiosyncrasy of my project
abnormal interruption of the server process may cause the socket continue occupying the port. Thus the server is not available until the zombie process is killed.

## e. Reused Code
I use sample code from "Beej's Guide to Network Programming" as template. My code is mostly developped from what he puts online. The copy part of his code is commentted in the code file. 

