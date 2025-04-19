#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "onenet_config.h"
#include "onenet_mqtt.h"

int onenet_init(onenet_client_t *client)
{
	assert(client != NULL);

	mosquitto_lib_init();				//初始化mosquitto库

	//直连设备属性上传主题
	snprintf(client->pub_topic_post, sizeof(client->pub_topic_post),
				"$sys/%s/%s/thing/property/post", ONENET_PRODUCT_ID, ONENET_DEVICE_ID);
	
	//直连设备属性上传回复主题
	snprintf(client->sub_topic_post_reply, sizeof(client->sub_topic_post_reply), 
				"$sys/%s/%s/thing/property/post/reply", ONENET_PRODUCT_ID, ONENET_DEVICE_ID);

	//设置直连设备属性主题
	snprintf(client->sub_topic_property_set, sizeof(client->sub_topic_property_set),
				"$sys/%s/%s/thing/property/set", ONENET_PRODUCT_ID, ONENET_DEVICE_ID);

	//设置直连设备属性回复主题
	snprintf(client->pub_topic_property_set_reply, sizeof(client->pub_topic_property_set_reply),
				"$sys/%s/%s/thing/property/set_reply", ONENET_PRODUCT_ID, ONENET_DEVICE_ID);
	
	//printf("pub_topic_post:%s\nsub_topic_post_reply:%s\n",client->pub_topic_post, client->sub_topic_post_reply);
	client->mosq = mosquitto_new(ONENET_DEVICE_ID, true, client);			//新建一个新的MQTT的客户端连接
	if(client->mosq == NULL)
		return -1;
	
	//设置客户端的username和passwd
	return mosquitto_username_pw_set(client->mosq, ONENET_PRODUCT_ID, ONENET_DEVICE_TOKEN);
}

int onenet_connect(onenet_client_t *client)
{
	assert(client != NULL);

	//连接到OneNET服务器
	return mosquitto_connect(client->mosq, ONENET_HOST, ONENET_PORT, 120);
}

int onenet_subscribe(onenet_client_t *client,
                     void (*on_message)(struct mosquitto *, void *, const struct mosquitto_message *))
{
	int ret = 0;

	assert(client != NULL);

	//设置收到消息时处理的回调函数
	mosquitto_message_callback_set(client->mosq, on_message);

	//订阅主题
	ret |= mosquitto_subscribe(client->mosq, NULL, client->sub_topic_post_reply, 0);
	ret |= mosquitto_subscribe(client->mosq, NULL, client->sub_topic_property_set, 0);

	return ret;
}

int onenet_publish(onenet_client_t *client, const char *payload)
{
	assert(client != NULL);

	//发布消息
	return mosquitto_publish(client->mosq, NULL, client->pub_topic_post, 
			strlen(payload), payload, 0, false);
}

int onenet_publish_set_reply(onenet_client_t *client, const char *payload)
{
	assert(client != NULL);

	//回复设置属性的主题
	return mosquitto_publish(client->mosq, NULL, client->pub_topic_property_set_reply, 
			strlen(payload), payload, 1, false);
}

void onenet_loop_start(onenet_client_t *client)
{
	assert(client != NULL);
	
	//启用网络事件循环处理线程，不阻塞主线程
	mosquitto_loop_start(client->mosq);
}

void onenet_disconnect(onenet_client_t *client)
{
	assert(client != NULL);

	mosquitto_loop_stop(client->mosq, true);			//停止网络循环
	mosquitto_disconnect(client->mosq);			//断开与服务器的连接
	mosquitto_destroy(client->mosq);			//销毁客户端实例
	mosquitto_lib_cleanup();					//清理mosquitto库的全局资源
}
