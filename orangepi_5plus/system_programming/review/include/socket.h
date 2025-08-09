#ifndef __SOCKET__H__
#define __SOCKET__H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define IPADDR "192.168.0.157"

extern char *getAddress(char *cmd);
extern void finish_commun(char *ip);
extern int initSocket(int sockfd, char *ip, char *port);
extern void out_addr(struct sockaddr_in *client_addr);
extern void do_service(int sockfd);

#endif
