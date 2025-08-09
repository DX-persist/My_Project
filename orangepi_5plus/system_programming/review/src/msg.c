#include "socket.h"
#include "msg.h"

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

//通过sockfd套接字写入数据，要写入的数据存放在buf中，buf只包括协议的体部
int writeMsg(int sockfd, void *buf, size_t len)
{
	Msg message;
	memset(&message, 0, sizeof(message));

	strcpy(message.head, "User-defined protocol");
	memcpy(message.buffer, buf, len);
	message.check_code = msgCheck(&message);	

	if(write(sockfd, &message, sizeof(message)) != sizeof(message)){
		perror("write error");	
		return -1;
	}else{
		return sizeof(message);
	}
}

//通过sockfd套接字获取数据，读取出的数据存放在buf中，buf只包括协议的体部
int readMsg(int sockfd, void *buf, size_t len)
{
	Msg message;
	memset(&message, 0, sizeof(message));
	size_t size;

	if((size = read(sockfd, &message, sizeof(message))) < 0){
		perror("read error");
		return -1;
	}else if(size == 0){
		return 0;
	}else{
		unsigned char s = msgCheck(&message);
		if(s == message.check_code 
					&& (!strcmp(message.head, "User-defined protocol"))){
			memcpy(buf, message.buffer, len);
			return sizeof(message);
		}else{
			return -1;
		}
	}
}

