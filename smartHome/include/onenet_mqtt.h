#ifndef __ONENET_MQTT_H__
#define __ONENET_MQTT_H__

#include <mosquitto.h>

typedef struct{
	char pub_topic_post[256];
	char pub_topic_property_set_reply[256];
	char sub_topic_post_reply[256];
	char sub_topic_property_set[256];
	struct mosquitto *mosq;
}onenet_client_t;

int onenet_init(onenet_client_t *client);
int onenet_connect(onenet_client_t *client);
int onenet_subscribe(onenet_client_t *client,
                     void (*on_message)(struct mosquitto *, void *, const struct mosquitto_message *));
int onenet_publish(onenet_client_t *client, const char *payload);
int onenet_publish_set_reply(onenet_client_t *client, const char *payload);
void onenet_loop_start(onenet_client_t *client);
void onenet_disconnect(onenet_client_t *client);

#endif
