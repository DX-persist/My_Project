#ifndef _ONENET_CONFIG_H
#define _ONENET_CONFIG_H

#include <mosquitto.h>
#include "dht22_config.h"

typedef struct{
	const char *deviceID;
	const char *productID;
	const char *token;
	const char *host;
	int port;
	int keepalive;
}onenet_config_t;

typedef struct{
	char pub_prop_topic[128];					//发布属性上报(property/post)
	char pub_prop_set_reply_topic[128];			//发布属性设置的回复(property/set_reply)
	char sub_prop_set_topic[128];				//订阅属性设置指令(property/set)
	char sub_prop_reply_topic[128];		//订阅上报属性后的响应(property/post_reply)
}onenet_topic_t;

typedef struct{
	char payload[512];
	size_t payloadlen;
	int qos;
	bool retain;
}pub_msg_t;

typedef struct{
	int ret;
	struct mosquitto *mosq;
	onenet_config_t config;
	onenet_topic_t topic;
	DHT22_Data_t Data;
	pub_msg_t pub_msg;
}onenet_client_t;

#define LED_PIN 0
#define CHECK_MOSQ_ERR(cond, failure_msg, success_msg, retval) 															\
	do{																													\
		if(cond){																										\
			fprintf(stderr, "%s|%s|%d %s: %s\n", __FILE__, __func__, __LINE__, failure_msg, mosquitto_strerror(cond));	\
			return retval;																								\
		}else{																											\
			fprintf(stdout, "%s|%s|%d %s\n", __FILE__, __func__, __LINE__, success_msg);								\
		}																												\
	}while(0)																											\

#endif
