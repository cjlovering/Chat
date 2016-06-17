all: server client testDriver

server: server.o tree.o
	gcc server.o tree.o -pthread -g -o server

server.o: server.h server.c
	gcc -pthread -g -c server.c

client: client.o
	gcc client.o  -pthread -g -o client

client.o: client.h  client.c
	gcc -pthread -g -c client.c

tree.o: tree.h tree.c
	gcc -pthread -g -c tree.c

testDriver: testDriver.o tree.o
	gcc testDriver.o tree.o -pthread -g -o testDriver

testDriver.o: testDriver.c
	gcc -pthread -g -c testDriver.c


clean:
	rm *.o
	rm server
	rm client
	rm testDriver
tar:
	tar README makefile server.c client.c tree.c tree.h chat.h
