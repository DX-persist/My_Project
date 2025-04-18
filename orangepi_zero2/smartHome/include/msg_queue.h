#ifndef __MSGQUEUE_H_
#define __MSGQUEUE_H_

//system defined
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>


#define QUEUE_NAME "/mq_queue"

extern mqd_t msg_queue_create(void);
extern int msg_queue_send(mqd_t mqd, void *msg_ptr, size_t msg_len);
extern void msg_queue_free(mqd_t mqd);

#endif