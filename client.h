#ifndef _CLIENT_H
#define _CLIENT_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <errno.h>
#include <signal.h>

#include <string.h>
#include  <signal.h>
#include "utility.h"

void changeName();

static volatile int keepRunning = 1;
void  inthandler(int sig);

#endif
