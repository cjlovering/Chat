#ifndef CHAT_H_
#define CHAT_H_

#include <semaphore.h>
#include <pthread.h>

#define MAX_CLIENTS (5)
pthread_mutex_t mutex;       /* mutex used to protect regions */
sem_t active_connections;            /* the semaphores to limit the number of clients */

#endif

