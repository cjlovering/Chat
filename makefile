all: server client

server: server.o
	gcc server.o -pthread  -o server

server.o: server.h server.c
	gcc -pthread -c server.c

client: client.o
	gcc client.o  -pthread -o client

client.o: client.c
	gcc -pthread -c client.c

clean:
	rm *.o
	rm server
	rm client

tar:
	tar README makefile server.c client.c chat.h
