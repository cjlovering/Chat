all: server client testDriver

server: server.o tree.o utility.o
	gcc server.o tree.o utility.o -pthread -g -o server

server.o: server.h server.c
	gcc -pthread -g -c server.c

client: client.o utility.o
	gcc client.o utility.o -pthread -g -o client

client.o: client.h  client.c
	gcc -pthread -g -c client.c

tree.o: tree.h tree.c
	gcc -pthread -g -c tree.c

testDriver: testDriver.o tree.o utility.o
	gcc testDriver.o tree.o utility.o -pthread -g -o testDriver

testDriver.o: testDriver.c
	gcc -pthread -g -c testDriver.c

utility.o: utility.h utility.c
	gcc -pthread -g -c utility.c

clean:
	rm *.o
	rm server
	rm client
	rm testDriver
tar:
	tar README makefile server.c client.c tree.c tree.h chat.h
