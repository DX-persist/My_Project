#ifndef __MSG__H__
#define __MSG__H__

typedef struct
{
	char head[22];					//定义协议的头部
	unsigned char check_code;		//定义校验码						
	char buffer[512];				//定义要读写数据的缓存区域				
}Msg;

//通过sockfd套接字写入数据，要写入的数据存放在buf中，buf只包括协议的体部
extern int writeMsg(int sockfd, void *buf, size_t len);

//通过sockfd套接字获取数据，读取出的数据存放在buf中，buf只包括协议的体部
extern int readMsg(int sockfd, void *buf, size_t len);

#endif
