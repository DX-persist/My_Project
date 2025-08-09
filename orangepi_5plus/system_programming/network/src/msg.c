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

int writeMsg(int sockfd, void *buf, size_t len)
{
    Msg message;
    memset(&message, 0, sizeof(message));
    strcpy(message.head, "User-defined protocol");
    memcpy(message.buffer, buf, len);
    message.check_code = msgCheck(&message);

    if(write(sockfd, &message, sizeof(message)) != sizeof(message)){
        return -1;
    }
    return sizeof(message);
}

int readMsg(int sockfd, void *buf, size_t len)
{
    Msg message;
    size_t size;
    memset(&message, 0, sizeof(message));
    
    if((size = read(sockfd, &message, sizeof(message))) < 0){
        return -1;
    }else if(size == 0){
        return 0;
    }

    unsigned char s = msgCheck(&message);
    if(s == message.check_code 
            && (!strcmp(message.head, "User-defined protocol"))){
        memcpy(buf, message.buffer, len);
        return sizeof(buf);
    }else{
        return -1;
    }
}