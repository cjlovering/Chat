#ifndef _CLIENT_H
#define _CLIENT_H

#include <pthread.h>

static volatile int keepRunning = 1;
void  inthandler(int sig);

#endif
