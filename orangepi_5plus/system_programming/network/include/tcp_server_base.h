#ifndef __BASE__H__
#define __BASE__H__

char *getAddress(char *cmd);
int initSocket(char *port, char *ip);
void out_addr(struct sockaddr_in *clientaddr);
void do_service(int sockfd);

#endif