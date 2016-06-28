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
#include "utility.h"


static volatile int running = 1;

/* function prototypes */
int prompt(int sock);
int parse (char* message, int sock);
void* connection (void *sock_void);
void* interactive(void *zero);
void sigint_handler(int sig); /* prototype */
void printUsage(void);
void destroyServer(void);
void sendAll(char* msg);
void sendAllHelper(char* msg, Node* n);
void sendAllBut(char* msg1, char* msg2, char* name);
void sendAllHelperBut(char* msg1, char* msg2, char* name, Node* n);
void populateList(char* list);
void populateListHelper(char* list, Node* n);
 
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
  char* leave = "_EXIT_";
  char* saveptr1;
  char* parsed[100] = {0};
  char* inputMessages = strtok_r(message, " ", &saveptr1);
  int i = 0;
  
  while (inputMessages != NULL)
  {
    parsed[i] = inputMessages;
    i++;
    inputMessages = strtok_r(NULL, " \r\t\n", &saveptr1);
  }

  if (parsed[0] == NULL) return 1;

  if ( strncmp(parsed[0], leave, strlen(leave)) == 0)
  {
    char* name = parsed[1];
    //    scanf("%s %s", key, name);
 
    printf("USER %s leaving\n", name);
    //client leaving
    //remove him from users
    User* u = findUser(users, name);
    if (u == NULL) printf("user not found");
    removeUser(users, u);
    
    //close the connection
    //let all the users know he's leaving

    char* userLeaving = malloc(sizeof(char) * 150);
    if (userLeaving ==  NULL)  error("malloc failed");
    
    strcat(userLeaving, ">>> ");
    strcat(userLeaving, name);
    strcat(userLeaving, " has left Chat.\n");    
    
    sendAll(userLeaving); 
    free(userLeaving);
    
    return 0;
  }
  else if ( strncmp(parsed[0], "list", strlen("list")) == 0 ||
	    strncmp(parsed[0], "l",  strlen("l")) == 0 )
  {
    char* listOfUsers = calloc(sizeof(char*), 10 + 22 * MAX_CLIENTS);
    populateList(listOfUsers);
    int i = send(sock, listOfUsers, strlen(listOfUsers), MSG_NOSIGNAL);
    if (i < 0) 
      error("ERROR reading from socket");
  }
  else 
  {
    //message all others
    char* reconstruct = calloc(sizeof(char), (20 + 255));
    char* reconstructBut = calloc(sizeof(char), (4 + 255));
    strcat(reconstruct, "\n");
    strcat(reconstructBut, "Me: ");

    strcat(reconstruct, parsed[0]);
    strcat(reconstruct, ": ");

    int i = 1;
    while(parsed[i]!=NULL&&strlen(trim(parsed[i]))!=0)
    {
      strcat(reconstruct, parsed[i]);
      strcat(reconstruct, " ");
      strcat(reconstructBut, parsed[i++]);
      strcat(reconstructBut, " ");
    }
    
    sendAllBut(reconstruct, reconstructBut, parsed[0]);

    free(reconstruct);
    free(reconstructBut);
  }

  return 1;
}

int prompt(int sock)
{
  int n;
  char* buffer = malloc(sizeof(char) * 255);
  bzero(buffer,256);
  n = read(sock, buffer, 255);
  
  if (n < 0) 
    error("ERROR reading from socket");
  buffer = trim( buffer );
  int value = parse( buffer, sock ); //0, stop. 1, continue.
  free(buffer);
  
  return value;
}


/**
 * This function will become a connection thread!
 * It will service the client, and handle the connection.
 * It will gracefully handle the end of the connection!
 */
void* connection (void *sock_void)
{
  int sock = (intptr_t)sock_void;
  while ( prompt ( sock ) ) {}
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
    else if (strcmpc(trim(buffer), "exit")  == 0 ||
	     strcmpc(trim(buffer), "quit")  == 0 ||
	     strcmpc(trim(buffer), "leave") == 0 ||
	     strcmpc(trim(buffer), "q")     == 0)
    {
      destroyServer();
      //continue;
    }
    else if (strcmpc(trim(buffer), "help")  == 0 ||
	     strcmpc(trim(buffer), "info")  == 0 ||
	     strcmpc(trim(buffer), "h")  == 0)
    {
      printUsage();
    }
    else if (strcmpc(trim(buffer), "list")  == 0 ||
	     strcmpc(trim(buffer), "l")  == 0 ||
	     strcmpc(trim(buffer), "users")  == 0)
    {
      printf("Active Users: ");
      displayTree(users);
      printf("\n");
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
  destroyServer();
}

void destroyServer(void)
{
  printf("Destroying server!\n");

  //tell users to leave
  sendAll("_SERVER_EXIT_");

  //free users

  //end threads

  printf("Done\n");
  exit(0);
}

void sendAll(char* msg)
{
  if(users != NULL) sendAllHelper(msg, users->root);
}

void sendAllHelper(char* msg, Node* n)
{
  if (n == NULL) return;

  int socket  = n->data->socket;
  
  int i = send(socket, msg, strlen(msg), MSG_NOSIGNAL);
  if (i < 0) error("ERROR reading from socket");

  sendAllHelper(msg, n->left);
  sendAllHelper(msg, n->right);
}

void sendAllBut(char* msg1, char* msg2, char* name)
{
  if(users != NULL) sendAllHelperBut(msg1, msg2, name,  users->root);
}

void sendAllHelperBut(char* msg1, char* msg2, char* name, Node* n)
{
  if (n == NULL) return;

  int socket  = n->data->socket;
  int i;

  if (strcmp(n->data->id, name)==0)
    i = send(socket, msg2, strlen(msg2), MSG_NOSIGNAL);
  else
    i = send(socket, msg1, strlen(msg1), MSG_NOSIGNAL);

  if (i < 0) error("ERROR reading from socket");

  sendAllHelperBut(msg1, msg2, name, n->left);
  sendAllHelperBut(msg1, msg2, name, n->right);
}

void populateList(char* list)
{
  if (users == NULL) return;
  
  populateListHelper(list, users->root);
}

void populateListHelper(char* list, Node* n)
{
  if (n == NULL) return;

  strcat(list, n->data->id);
  strcat(list, " ");
 
  populateListHelper(list, n->left);
  populateListHelper(list, n->right);
}

void printUsage(void)
{
  printf("---------------------------------------------\n");
  printf("This is the Server for the Chat.\n");
  printf("info / help / h      = printUsage\n");
  printf("quit / exit / leave / q  = ends service\n");
  printf("list / l                 = shows all users\n"); 
  printf("---------------------------------------------\n");
}
