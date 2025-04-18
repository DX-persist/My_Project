#ifndef __GLOBAL__H
#define __GLOBAL__H

#include <mqueue.h>
typedef struct
{
    mqd_t mqd;
    struct control *ctrl_phead;
}ctrl_info_t;

#define BUFFER_SIZE 6

#endif