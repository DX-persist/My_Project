#ifndef __MSG__H__
#define __MSG__H__

typedef struct 
{
    char head[22];                      //协议的头部
    unsigned char check_code;           //校验码
    char buffer[512];                   //协议的体部
}Msg;

//向sockfd写入数据，要写入的数据存放在buf中
extern int writeMsg(int sockfd, void *buf, size_t len);
//从sockfd中读取数据，读取的数据放到buf中
extern int readMsg(int sockfd, void *buf, size_t len);

#endif