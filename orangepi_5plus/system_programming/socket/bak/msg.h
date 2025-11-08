#ifndef __MSG_H__
#define __MSG_H__

#include <stddef.h>

typedef struct{
	char head[48];
	char check_num;
	char buffer[512];
}Msg_t;

extern int write_msg(int sockfd, char *buff, size_t len);
extern int read_msg(int sockfd, char *buff, size_t len);

#endif
