#ifndef __SOCKET__H__
#define __SOCKET__H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>

#define IPADDR "192.168.0.157"

extern char *getAddress(char *cmd);
extern void finishCom(char *ip);
extern void out_addr(struct sockaddr_in *clientaddr);

#endif
