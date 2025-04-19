#include "msg_queue.h"

mqd_t msg_queue_create(void)
{
    mqd_t mqd = -1;
    struct mq_attr attr;

    attr.mq_flags = 0;          //设置默认的标志位为阻塞模式
    attr.mq_maxmsg = 10;        
    //消息队列中最多有10条消息，如果超过此数目仍然往消息队列中
    //放消息就会报错EAGAIN
    attr.mq_msgsize = 256;      //每条消息的最大字节数是256个字节
    attr.mq_curmsgs = 0;        //当前队列中的消息数是0条

    mqd = mq_open(QUEUE_NAME, O_RDWR | O_CREAT, 0666, &attr);
    printf("%s |%s|%d mqd = %d\n",__FILE__,__func__,__LINE__,mqd);

    return mqd;
}

int msg_queue_send(mqd_t mqd, void *msg_ptr, size_t msg_len)
{
    int send_retval = -1;

    send_retval = mq_send(mqd, (char *)msg_ptr, msg_len, 0);

    return send_retval;
}

void msg_queue_free(mqd_t mqd)
{
    if(mqd != (mqd_t)-1)
    {
        mq_close(mqd);
        mq_unlink(QUEUE_NAME);
        mqd = -1;
    }
}