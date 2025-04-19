#ifndef __SOCKET_H
#define __SOCKET_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netinet/tcp.h>

#define IPADDR "192.168.0.6"
//#define IPADDR "192.168.43.7"
#define IPPORT "8192"

int socket_init(const char *ipaddr,const char *ipport);

#endif
