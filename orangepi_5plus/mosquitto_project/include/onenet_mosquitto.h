#ifndef _ONENET_MOSQUITTO_H
#define _ONENET_MOSQUITTO_H 

#include "onenet_config.h"

extern int onenet_init(onenet_client_t *client);
extern void onenet_build_topics(onenet_client_t *client);
extern int onenet_connect(onenet_client_t *client);
extern int onenet_loop_start(onenet_client_t *client);
extern void on_connect(struct mosquitto *mosq, void *userdata, int rc);
extern void on_subscribe(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos);
extern void onenet_callback_set(onenet_client_t *client);
extern int onenet_disconnect(onenet_client_t *client);
 
#endif
