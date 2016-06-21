#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include  <signal.h>
#include "client.h"

void error(const char *msg);
void inthandler(int sig);
void* writer(void* user);
void* reader(void* user);
int verify(char* username);


int sockfd;
static volatile int running = 1;

int main(int argc, char *argv[])
{
  int portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  char buffer[256];
  if (argc < 3) {
    fprintf(stderr,"usage %s hostname port\n", argv[0]);
    exit(0);
  }
  portno = atoi(argv[2]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");
  server = gethostbyname(argv[1]);

  if (server == NULL) 
    {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
    }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    error("ERROR connecting");


  char username[20];

  printf("Enter user name: ");
  scanf("%s", username);

  if ( !verify(username) )
  {
    printf("INVALID USER\nExiting now...");
    return 0;
  }
  else
  {
    printf("Hi %s! Welcome to that chat!\n", username);
  }

  signal(SIGINT, inthandler);

  pthread_t threads[2];

  if( pthread_create( &threads[0], NULL, writer, (void *)username ) != 0 )
  {
    perror("pthread_create");
    exit(1);
  }

  if( pthread_create( &threads[1], NULL, reader, (void*)0 ) != 0 )
  {
    perror("pthread_create");
    exit(1);
  }

  while(running){}//do stuff in threads

  close(sockfd);
  return 0;
}

void* writer(void* username)
{
  char buffer[256];
  int n;

  while(1)
    {
      printf("--> ");
      bzero(buffer,256);
      fgets(buffer,255,stdin);
      if((buffer[0] == '\n')|(strlen(buffer) == 0)){
	continue;
      }
      n = write(sockfd,buffer,strlen(buffer));
      if (n < 0) 
	error("ERROR writing to socket");
    }  
}

void* reader(void* null)
{
  char buffer[256];
  int n;
  
  while(1)
    {
      bzero(buffer, 256);
      n = read(sockfd,buffer,255);
      if (n < 0)
        error("ERROR reading from socket");
      printf("%s\n",buffer);
    }
}

int verify( char* username )
{
  char*  buffer[256];
  //write name to server
  int n = write(sockfd,buffer,strlen(username));

  if (n < 0) 
    error("ERROR writing to socket");

  //wait for confirmation from server that its a valid name
  bzero(buffer, 256);
  
  int valid;
  n = read(sockfd , &valid, sizeof(int));
  
  //n = read(sockfd,buffer,255);
  if (n < 0)
    error("ERROR reading from socket");

  return valid;
}

void error(const char *msg)
{
  perror(msg);
  exit(0);
}

void inthandler(int sig)
{
  printf("Leaving server...\n");
  running = 0;
}
