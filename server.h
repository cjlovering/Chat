#ifndef SERVER_H_
#define SERVER_H_

/* headers */
#include <semaphore.h>
#include <pthread.h>
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

/* function prototypes */
int prompt(int sock);
int parse (char* message, int sock);
void* connection (void *sock_void);
void* interactive(void *zero);
void sigint_handler(int sig);
void printUsage(void);
void destroyServer(void);
void sendAll(char* msg);
void sendAllHelper(char* msg, Node* n);
void sendAllBut(char* msg1, char* msg2, char* name);
void sendAllHelperBut(char* msg1, char* msg2, char* name, Node* n);
void populateList(char* list);
void populateListHelper(char* list, Node* n);
                                                  
/* variables & constants */
#define MAX_CLIENTS (100)
pthread_mutex_t mutex;       /* mutex used to protect regions */
sem_t active_connections;            /* the semaphores to limit the number of clients */
Tree* users;
pthread_t* threads;
static int thread_counter = 0;
static int thread_size = 10;
static volatile int running = 1;

#endif

