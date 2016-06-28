#include "client.h"

void* writer(void* user);
void* reader(void* user);
int verify(char* username);
void printUsage(void);
void closeClientConnection(void);
char* formMessage(char* msg);
void leaveServer();

int sockfd;
char* username;
static volatile int running = 1;
void sigint_handler(int sig);
pthread_t threads[2];


int main(int argc, char *argv[])
{
  int portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  char buffer[256];
  if (argc < 3)
  {
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

  /* handling interrupts */
  struct sigaction sa;
  
  sa.sa_handler = sigint_handler;
  sa.sa_flags = 0; // or SA_RESTART
  sigemptyset(&sa.sa_mask);
    
  if ( sigaction(SIGINT, &sa, NULL) == -1 ) 
  {
    perror("sigaction");
    exit(1);
  }
    
  /*  verification */
  username = calloc(sizeof(char), 20);
  printf(" _________________________________________\n");
  printf("| Enter user name: ");
  scanf("%s", username);

  if ( !verify(username) )
  {
    printf("INVALID USER\n|  exiting now...");
    printf("|_________________________________________\n");
    running = 0;
    exit(0);
  }
  else
  {
    printf("| Hi %s! Welcome to that chat!\n", username);
    fflush(stdout);
  }

 
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

  while(running){}//without this, everythings dies!
  close(sockfd);
  return 0;
}

void* writer(void* username)
{
//  char* buffer[256];// = calloc(sizeof(char), 256);
  //it
//  char* workingBuffer;// = calloc(sizeof(char), 256);
  int n;
  int first = 0;

  while(running)
  {
    char* buffer = calloc(sizeof(char), 256);
    char* toBeFreed = buffer;
    
    if (first) printf("|> ");
    else first = 1;
    bzero(buffer,256);
    fgets(buffer,255,stdin);

    //workingBuffer = calloc(sizeof(char), 256);
    // strcat(workingBuffer, trim(buffer));
    
    buffer = trim(buffer);

    if(strlen(buffer) == 0)
      continue;

    else if (strcmpc(trim(buffer), "exit")  == 0 ||
             strcmpc(trim(buffer), "quit")  == 0 ||
             strcmpc(trim(buffer), "leave") == 0 ||
             strcmpc(trim(buffer), "q")     == 0)
    {
      printf("| exiting now!\n");
      printf("|_________________________________________\n");
      running = 0;

      continue;                                                            
    }
    else if (strcmpc(trim(buffer), "help")  == 0 ||
             strcmpc(trim(buffer), "info")  == 0 ||
             strcmpc(trim(buffer), "h")  == 0)
    {
      printUsage();
      continue;
    }
    else if (strcmpc(trim(buffer), "list")  == 0 ||
	     strcmpc(trim(buffer), "users")  == 0 ||
             strcmpc(trim(buffer), "l")  == 0)
    {
      n = send(sockfd,"list",strlen("list"), MSG_NOSIGNAL);
      if (n < 0) 
	error("ERROR writing to socket");
      
      continue;
    }

    char* newName = calloc(sizeof(char), 21);
    char* garb = calloc(sizeof(char), 21);
    int c = sscanf(buffer,  "%s %s\n", garb, newName);
    
    if(c == 2 && strcmp(garb, "change")  == 0 )
    {
      changeName(newName);
      free(newName);
      free(garb);
      continue;
    }
    else
    {
      free(newName);
      free(garb);
    }

    printf("####");
    //normal chat message
    char* bufferMessage = calloc(sizeof(char), (20 + 255));
    strcat(bufferMessage, username);
    strcat(bufferMessage, " ");
    strcat(bufferMessage, buffer);
    
    n = send(sockfd,bufferMessage,strlen(bufferMessage), MSG_NOSIGNAL);
    
    if (n < 0) 
      error("ERROR writing to socket");

    free(bufferMessage);
    free(toBeFreed);
  }  

  //handle exit

  leaveServer();
  closeClientConnection();
}

void leaveServer()
{
  char* leave = calloc(sizeof(char), (20 + 10));
  strncat(leave, "_EXIT_ ", sizeof("_EXIT_ "));
  strcat(leave, username);
  int n = send(sockfd, leave, strlen(leave), MSG_NOSIGNAL);
  if (n < 0)
        error("ERROR writing to socket");
  free(leave);
}

void changeName(char* newName)
{
  
  char* leave = calloc(sizeof(char), 255);
  strcat(leave, username);
  strncat(leave, " change ", sizeof(" change "));
  strcat(leave, newName);
  
  int n = send(sockfd, leave, strlen(leave), MSG_NOSIGNAL);
  if (n < 0)
    error("ERROR writing to socket");
  
  //wait for confirmation from server that its a valid name    
  free(leave);
}

void* reader(void* null)
{
  char buffer[256];
  int n;
  
  while(running)
  {
    bzero(buffer, 256);
    n = recv(sockfd,buffer,255,0);
    if (n < 0)
      error("ERROR reading from socket");
    
    char* leave = "_SERVER_EXIT_";
    if ( strncmp(buffer, leave, strlen(leave)) == 0 )
    {
      printf("| \nServer went down, exiting...\n");
      closeClientConnection();
    }

    char* newName = calloc(sizeof(char), 20);
    int c = sscanf(buffer, "| Name Change Successful. Your name is now: %s\n", newName);
    if(c > 0)
    {
      free(username);
      username = calloc(sizeof(char), 20);
      strcat(username, newName);//username = newName;
    }
    free(newName);
    printf("%s\n",buffer);
  }
}

int verify( char* username )
{
  //write name to server
  int n = send(sockfd,username,strlen(username), MSG_NOSIGNAL);

  if (n < 0) 
    error("ERROR writing to socket");

  //wait for confirmation from server that its a valid name  
  int valid;
  n = recv(sockfd , &valid, sizeof(int), 0);
  if (n < 0)
    error("ERROR reading from socket");

  return valid;
}

void sigint_handler(int sig)
{
  printf("| Leaving Chat...\n");
  leaveServer();
  closeClientConnection();
}

void closeClientConnection(void)
{
//  printf("| Connection terminated.\n");
  exit(0);
}

void printUsage(void)
{
  printf("|---------------------------------------------\n");
  printf("| Hi %s! Welcome to the Chat.\n", username);
  printf("| info / help / h            = printUsage\n");
  printf("| quit / exit / leave / q    = ends service\n");
  printf("| list / l / users           = shows all clients\n");
  printf("| \"message\"                 = sends to everyone in room\n");
  printf("| change NEW_NAME            = attempts to change username\n");
  printf("| whisper/w USER \"message\" = to send secret messages\n");
  printf("|---------------------------------------------\n");
}

char* formMessage(char* msg)
{
  char* result = malloc(sizeof(char) * (20 + 255));
  strcpy(result, username);
  strncat(result, " ", 1);
  strncat(result, msg, 255);
  return result;
}


