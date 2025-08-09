#include "msg.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

static unsigned char msgCheck(Msg *message)
{
	unsigned char s = 0;
	int i;

	for(i = 0; i < sizeof(message->head); i++){
		s += message->head[i];
	}
	for(i = 0; i < sizeof(message->buffer); i++){
		s += message->buffer[i];
	}

	return s;	
}

//向套接字中写入内容，要写的内容放在buf结构体中
int writeMsg(int sockfd, void *buf, size_t len)
{
	Msg message;
	memset(&message, 0, sizeof(message));

	strcpy(message.head, "User-defined protocol");
	memcpy(message.buffer, buf, len);
	message.checknum = msgCheck(&message); 

	if(write(sockfd, &message, sizeof(message)) != sizeof(message))
	{
		perror("write error");
		return -1;
	}
	return sizeof(message);
}
//从套接字中读取内容，然后将内容存放到buf中
int readMsg(int sockfd, void *buf, size_t len)
{
	Msg message;
	memset(&message, 0, sizeof(message));
	size_t size;

	if((size = read(sockfd, &message, sizeof(message))) < 0){
		perror("read error");
		return -1;
	}
	else if(size == 0){
		return 0;
	}
	
	unsigned char s = msgCheck(&message);
	if(s == message.checknum && 
				(!strcmp(message.head, "User-defined protocol")))
	{
		memcpy(buf, message.buffer, len);
		return len;
	}
	else
	{
		return -1;
	}
}
