#ifndef __ONENET__H
#define __ONENET__H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <mosquitto.h>

extern char pub_topic_post[256];
extern char sub_topic_post_reply[256];

struct mosquitto *OneNET_Connect(void);
void publish_message(struct mosquitto *mosq, const char *topic, const char *buffer);
void OneNET_Disconnect(struct mosquitto *mosq);
//int OneNET_Pub_Sub(struct mosquitto *mosq);

#endif