/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"
#include "tree.h"

/* function prototypes */
int prompt(int sock);
int parse (char* message, int sock);
void* connection (void *sock_void);


void error(const char *msg)
{
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno, pid;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  if (argc < 2) {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if ( bind(sockfd, (struct sockaddr *) &serv_addr,
	    sizeof(serv_addr)) < 0 ) 
    error("ERROR on binding");
  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  
  /* init the semaphore for active connections */
  if ( sem_init(&active_connections, 0, MAX_CLIENTS) < 0 )
    {
      perror("sem_init");
      exit(1);
    }

  /* create the mutex to lock regions of code */
  if (pthread_mutex_init(&mutex, NULL) < 0) 
    {
      perror("pthread_mutex_init");
      exit(1);
    } 

  while (1) {
    sem_wait(&active_connections); //

    /* block on  new client */
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    
    if (newsockfd < 0) 
      error("ERROR on accept");
    
    pthread_t current_thread;

    //TODO: implement  -->
    //threads.push( current_thread );

    if( pthread_create( &current_thread, NULL, connection, (void *)newsockfd ) != 0 )
      {
	perror("pthread_create");
	exit(1);
      }
    
  } /* end of while */

  close(sockfd);
  return 0; /* we never get here */
}


int parse (char* message, int sock)
{
  int n;
  printf("Got: %s", message);
  n = write(sock,"message recieved",15);
  if (n < 0) error("ERROR writing to socket");
  return 1;
}

int prompt(int sock)
{
  int n;
  char buffer[256];
      
  bzero(buffer,256);
  n = read(sock, buffer, 255);
  if (n < 0) error("ERROR reading from socket");
  
  return parse( buffer, sock ); //0, stop. 1, continue.
}


/**
 * This function will become a connection thread!
 * It will service the client, and handle the connection.
 * It will gracefully handle the end of the connection!
 */
void* connection (void *sock_void)
{
  int sock = (long) sock_void;
  while (prompt ( sock )){}
  close(sock);  //maybe don't want to do this.
  sem_post(&active_connections); //release spot in the queue
}

/* mutex functions */
void BeginRegion()
{
  pthread_mutex_lock(&mutex);
}

void EndRegion()
{
  pthread_mutex_unlock(&mutex);
}

