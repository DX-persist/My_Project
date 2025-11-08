#include "msg.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>

static int msg_check(Msg_t *message)
{
	unsigned char s = 0;

	//将消息头部和消息体中所有的数据累加作为校验码
	for(int i = 0; i < sizeof(message->head); i++){
		s += message->head[i];
	}
	for(int i = 0; i < sizeof(message->payload); i++){
		s += message->payload[i];
	}
	return s;
}

int write_msg(int sockfd, void *buf, size_t len)
{
	Msg_t message;
	char *title = "User-defined";

	//清空消息结构体
	memset(&message, '\0', sizeof(message));
	//填充消息头部
	strncpy(message.head, title, sizeof(message.head));
	//填充消息体
	memcpy(message.payload, buf, len);
	//填充消息校验码
	message.check_num = msg_check(&message);

	if(write(sockfd, &message, sizeof(message)) != sizeof(message)){
		perror("write error");
		return -1;
	}
	return sizeof(message);
}

int read_msg(int sockfd, void *buf, size_t len)
{
	Msg_t message;
	ssize_t bytes_read;

	//清空消息结构体
	memset(&message, '\0', sizeof(message));
	if((bytes_read = read(sockfd, &message, sizeof(message))) < 0){
		perror("read error");
		return -1;
	}else if(bytes_read == 0){
		return 0;
	}else{
		if(message.check_num == msg_check(&message) && 
				(!strcmp(message.head, "User-defined"))){
			memcpy(buf, message.payload, len);
		}
		return sizeof(message);
	}
}

