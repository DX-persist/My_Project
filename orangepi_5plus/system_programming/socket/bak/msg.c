#include "msg.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static int check_func(Msg_t *message)
{
	char s = 0;

	for(int i = 0; i < sizeof(message->head); i++){
		s += message->head[i];
	}

	for(int i = 0; i < sizeof(message->buffer); i++){
		s += message->buffer[i];
	}
	return s;
}

int write_msg(int sockfd, char *buff, size_t len)
{
	Msg_t message;
	memset(&message, '\0', sizeof(message));

	time_t now = time(NULL);
	char time_buffer[24];

	memset(time_buffer, '\0', sizeof(time_buffer));
	strncpy(time_buffer, ctime(&now), sizeof(time_buffer));
	sprintf(message.head, "%s:User defined protocol\n",time_buffer);

	memcpy(message.buffer, buff, sizeof(message.buffer));
	
	message.check_num = check_func(&message);

	if(write(sockfd, &message, sizeof(message)) != sizeof(message)){
		perror("write error");
		return -1;
	}else{
		return sizeof(message);
	}
}

int read_msg(int sockfd, char *buff, size_t len)
{
	Msg_t message;
	memset(&message, '\0', sizeof(message));
	size_t bytes_read;

	if((bytes_read = read(sockfd, &message, sizeof(message))) < 0){
		perror("read error");
		return -1;
	}else if(bytes_read == 0){
		return 0;
	}

	if(message.check_num == check_func(&message)){
		memcpy(buff, message.buffer, len);
		return sizeof(message);
	}
}

