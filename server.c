/* A simple server in the internet domain usin#include <stdio.h> TCP
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

#include <errno.h>
#include <signal.h>

#include "server.h"
#include "tree.h"

static volatile int running = 1;

/* function prototypes */
int prompt(int sock);
int parse (char* message, int sock);
void* connection (void *sock_void);
void* interactive(void *zero);
char* trim(char* str);
void sigint_handler(int sig); /* prototype */
 
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

  /* handling signals */
  struct sigaction sa;
  sa.sa_handler = sigint_handler;
  sa.sa_flags = 0; // or SA_RESTART
  sigemptyset(&sa.sa_mask);
  
  if (sigaction(SIGINT, &sa, NULL) == -1) 
  {
    perror("sigaction");
    exit(1);
  }

  pthread_t interactive_thread;
  /* create an interactive cli for server */
  if ( pthread_create( &interactive_thread, NULL, interactive, (void*)(intptr_t)0 ) != 0 )
  {
    perror("pthread_create");
    exit(1);
  }
  

  /* create structure for keeping track of users */
  users = newTree();
  
  while (running) 
  {    
    sem_wait( &active_connections ); //

    /* block on  new client */
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    
    if (newsockfd < 0) 
      error("ERROR on accept");
    
    pthread_t current_thread;

    // get user name and validate!
    char* userbuffer = malloc(sizeof(char) * 256);//[256];    
    bzero(userbuffer,256);
    int n = read(newsockfd, userbuffer, 255);
    if (n < 0) error("ERROR reading from socket");

    printf("New user %s attempting to join...\n", userbuffer);
    
    //determine if user is unique
    int valid;
    if ( valid = validate(users, userbuffer) )
    {
      //valid = 1
      int z = write( newsockfd, &valid, sizeof(int));
      if (z < 0) error("ERROR writing to socket");

      //create and add user
      addUser( users, userbuffer, newsockfd );

      //user valid and added
      printf("User %s valid and added!\n", userbuffer);
    }
    else
    {
      //valid = 0;
      free(userbuffer);
      sem_post(&active_connections); //release spot in the queue
      close(newsockfd);
      printf("User %s is invalid and connection was dropped.\n", userbuffer);
      continue;
      //for now just end it!
      //continue;
    }
   
    //TODO: implement  -->
    //threads.push( current_thread );

    if( pthread_create( &current_thread, NULL, connection, (void*)(intptr_t)newsockfd ) != 0 )
    {
      perror("pthread_create");
      exit(1);
    }
    
  } /* end of while */

  //clean up
  //wait on all threads closing


  close(sockfd);
  return 0; /* we never get here */
}


int parse (char* message, int sock)
{
  int n;
  printf("Got: %s", message);
  n = send(sock,"message recieved",15, MSG_NOSIGNAL);
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
  int sock = (intptr_t)sock_void;
  while (running){ prompt ( sock ); }
  printf("closing connection");
  close(sock);  //maybe don't want to do this.
  sem_post(&active_connections); //release spot in the queue
}

void* interactive(void *zero)
{
  char buffer[256];

  while(running)
  {
    printf("--> ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    if((buffer[0] == '\n')|(strlen(buffer) == 0))
      continue;
    printf("input = %s\n", buffer);
    if (strcmp(trim(buffer), "exit") == 0)
    {
      printf("exiting now!\n");
      running = 0;
      continue;
    }
  }

 //close all threads

 //end all clients


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

void sigint_handler(int sig)
{
  printf("Ignoring interrupt. If you want to kill the server, enter: exit");
}

char* trim(char* str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = '\0';

  return str;
}
