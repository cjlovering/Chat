#include "server.h"

int main(int argc, char *argv[])
{

  /* building socket */
  int sockfd, newsockfd, portno, pid;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  if (argc < 2)
  {
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
  if ( bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ) 
    error("ERROR on binding");
  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  
  /* init the semaphore for active connections. */
  if ( sem_init(&active_connections, 0, MAX_CLIENTS) < 0 )
  {
    perror("sem_init");
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


  /* create structure for keeping track of threads */
  threads = malloc(sizeof(pthread_t) * thread_size);

  /* create structure for keeping track of users */
  users = newTree();

  /* core loop handling connecting clients */
  /* thus, the original thread is the commissioner */
  while (running) 
  {    
    sem_wait( &active_connections );
    //block on there being room to connect, used to limit number of clients.
    //this isn't needed, but its a clean way of limiting the users, and keeping the
    //performance high. (this is an exclusive chat room.)
    /* init the semaphore for active connections. */
    if ( sem_init(&active_connections, 0, MAX_CLIENTS) < 0 )
    {
      perror("sem_init");
      exit(1);
    }       
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

    printf("| New user %s attempting to join...", userbuffer);
    
    //determine if user is unique
    int valid;
    if ( valid = validate(users, userbuffer) )
    {
      //valid = 1
      int z = write( newsockfd, &valid, sizeof(int));
      if (z < 0) error("ERROR writing to socket");

      //create and add user to tree stucture
      addUser( users, userbuffer, newsockfd );
     
      //user valid and added
      printf("User %s valid and added!\n", userbuffer);

      //inform the people
      addUser(users, userbuffer, newsockfd);
      
      char* newUser = calloc(sizeof(char), 40);
      char* others  = calloc(sizeof(char), 155);

      strcat(newUser, "| Others in the chat see you joined!");
      strcat(others, "\n| New User: ");
      strcat(others, userbuffer);
      strcat(others, " is now in the room!");
      
      sendAllBut(others, newUser, userbuffer);
      
      free(others);
      free(newUser);
    }
    else
    {
      //valid = 0;
      free(userbuffer);
      sem_post(&active_connections); //release spot in the queue
      close(newsockfd);
      printf("User %s is invalid and connection was dropped.\n", userbuffer);
      continue;
      //continue;
    }
  
    if( pthread_create( &current_thread, NULL, connection, (void*)(intptr_t)newsockfd ) != 0 )
    {
      perror("pthread_create");
      exit(1);
    }

      //store  thread in resizing array - gonna keep it simple.
      //threads can close themselves properly, but if the server is the one that exits
      //going to manually close all threads 

      if (thread_counter < thread_size)
      {
	threads[thread_counter++] = current_thread;
      }
      else
      {
	thread_size *=  2;
	threads = realloc(threads, thread_size * sizeof(pthread_t));
	threads[thread_counter++] = current_thread;
      }
      
  } /* end of while */

  return 0; 
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
  close(sock);  //maybe don't want to do this.
  sem_post(&active_connections); //release spot in the queue
}

/**
 * prompt :: int -> int
 * handles the prompting of clients, and their requests
 */
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
 * prompt :: int -> int
 * parses messages from clients and appropriately handles them
 */
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
 
    printf("User is %s leaving.\n", name);

    //remove him from users
    User* u = findUser(users, name);
    if (u == NULL)
    {
      printf("user not found");
      return 0;
    }

    //closing the connection!
    close(u->socket);

    //deleting user from tree
    deleteUser(users, name);
    
    //close the connection
    //let all the users know he's leaving
    char* userLeaving = calloc(sizeof(char), 150);
    if (userLeaving ==  NULL)  error("malloc failed");
    
    strcat(userLeaving, "\n| ");
    strcat(userLeaving, name);
    strcat(userLeaving, " has left Chat.");    
    
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
  else if ( strncmp(parsed[1], "change", strlen("change")) == 0)
  {
    char* newName = parsed[2];
    int valid;

    if (findUser(users, newName) != NULL)
    {
      int i = send(sock, "This name is taken.", strlen("This name is taken."), MSG_NOSIGNAL);
      if (i < 0) 
	error("ERROR reading from socket");

      return 1;
    }

    deleteUser(users, parsed[0]);
    addUser(users, newName, sock);

    char* msgMe = calloc(sizeof(char), 124);
    char* informOthers = calloc(sizeof(char), 124);

    strcat(msgMe, "| Name Change Successful. Your name is now: ");
    strcat(msgMe, newName);
    strcat(informOthers, "| Name Change: ");
    strcat(informOthers, parsed[0]);
    strcat(informOthers, " is now ");
    strcat(informOthers, newName);
      
    sendAllBut(informOthers, msgMe, newName);
    
    free(msgMe);
    free(informOthers);
    //inform other users of name change

    //inform success
    
  }
  else if ( strncmp(parsed[1], "whisper", strlen("whisper")) == 0)
  {
    //create message
    char* secret = calloc(sizeof(char), 255+25);
    char* from   = parsed[0];
    char* to     = parsed[2];

    //error checking
    if (secret == NULL || to == NULL)
    {
      char* errorMsg = "| malformed whisper request.";
      send(sock, errorMsg, strlen(errorMsg), MSG_NOSIGNAL);
      return 1;
    }
    
    //whisper to yourself?
    if (strcmp(to, from) == 0)
    {
      char* confirmation = calloc(sizeof(char), 100);
      strcat(confirmation, "| Note to self: ");

      int index = 3;
      while(parsed[index]!=NULL&&strlen(trim(parsed[index]))!=0)
      {
	strcat(confirmation, parsed[index++]);
	strcat(confirmation, " ");
      }

      send(sock, confirmation, strlen(confirmation), MSG_NOSIGNAL);

      free(confirmation);
      return 1;
    }
   
    User* u = findUser(users, to);

    if (u != NULL)
    {
      //build message for TO user
      strcat(secret, "\nWhisper from ");
      strcat(secret, from);
      strcat(secret, ": ");
    
      int index = 3;
      while(parsed[index]!=NULL&&strlen(trim(parsed[index]))!=0)
      {
	strcat(secret, parsed[index++]);
	strcat(secret, " ");
      }

      //send the whisper
      int j = send(u->socket, secret, strlen(secret), MSG_NOSIGNAL);
      if (j < 0) 
	error("ERROR reading from socket");
      
      //build confirmation message for FROM user
      char* confirmation = calloc(sizeof(char), 40);
      strcat(confirmation, "Whisper to ");
      strcat(confirmation, to);
      strcat(confirmation, " successful...");

      //send confirmation
      send(sock, confirmation, strlen(confirmation), MSG_NOSIGNAL);
      free(confirmation);
    } 
    else
    {
      //notify the FROM user that the TO user could not be found
      int j = send(sock, "| User not found", strlen("| User not found"), MSG_NOSIGNAL);
      if (j < 0) 
	error("ERROR reading from socket");      
    }
    free(secret);
  }
  else 
  {
    //build chat room messages
    char* reconstruct = calloc(sizeof(char), (20 + 255));
    char* reconstructBut = calloc(sizeof(char), (4 + 255));
    strcat(reconstruct, "\n");
    strcat(reconstructBut, "| Me: ");

    strcat(reconstruct, "| ");
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

/**
 * interactive :: void* -> void*
 * interactive thread for server admin to inspect chat
 */
void* interactive(void *null)
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
}

/**
 * sigint_handler :: int -> void
 * replaces normal keyboard interrupt to gracefully end server
 */
void sigint_handler(int sig)
{
  destroyServer();
}

/**
 * destroyServer :: -> void
 * dismantles chat service, canceling all threads and freeing all data
 */
void destroyServer(void)
{
  printf("Ending service...\n");

  //tell users to leave
  sendAll("_SERVER_EXIT_");

  //free users
  printf("Freeing users...\n");
  deleteTree(users);

  //end threads
  printf("Canceling threads!\n");
  int i = 0;
  while( i < thread_counter )
  {
    printf("Canceling thread %d...\n", i);
    pthread_cancel(threads[i++]);
  }
  
  printf("Threads canceled.\n");  

  printf("Done\n");
  exit(0);
}

void printUsage(void)
{
  printf("---------------------------------------------\n");
  printf("This is the Server for the Chat.\n");
  printf("info / help / h          = printUsage\n");
  printf("quit / exit / leave / q  = ends service\n");
  printf("list / l / userse        = shows all users\n"); 
  printf("---------------------------------------------\n");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// tree functionality functions that seemed to belong more here than in tree.c //

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

  if (i < 0) error("ERROR writing to socket");

  sendAllHelperBut(msg1, msg2, name, n->left);
  sendAllHelperBut(msg1, msg2, name, n->right);
}

void populateList(char* list)
{
  if (users == NULL) return;
  strcat(list, "| Active Users:  ");
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

