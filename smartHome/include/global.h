#ifndef __GLOBAL__H
#define __GLOBAL__H

#include <mqueue.h>
#include "onenet_mqtt.h"

typedef struct
{
    mqd_t mqd;
    struct control *ctrl_phead;
    onenet_client_t *mqtt_client;
}ctrl_info_t;

#define BUFFER_SIZE 6

#endif