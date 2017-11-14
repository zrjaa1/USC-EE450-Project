#build an executable named aws from aws.c
#TARGET = serverA serverB serverC aws client

all: serverA.c serverB.c serverC.c aws.c client.c
	gcc -o serverA serverA.c
	gcc -o serverB serverB.c
	gcc -o serverC serverC.c
	gcc -o aws aws.c
	gcc -o client client.c

serverA: serverA.o
	./serverA

serverB: serverB.o
	./serverB

serverC: serverC.o
	./serverC

aws: aws.o
	./aws

clean: 
	$(RM) serverA
	$(RM) serverB
	$(RM) serverC
	$(RM) aws
	$(RM) client
	$(RM) serverA.o
	$(RM) serverB.o
	$(RM) serverC.o
	$(RM) aws.o
	$(RM) client.o
	
