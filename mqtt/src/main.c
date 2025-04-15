#include "onenet_mqtt.h"
#include "cJSON_Parse.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
    assert(userdata != NULL);
    onenet_client_t *client = (onenet_client_t *)userdata;

    printf("[RECV] Topic: %s\nPayload: %s\n", msg->topic, (char *)msg->payload);

    if(strstr(msg->topic, "/thing/property/post/reply")){
        printf("✅ 平台收到post回复(上传确认)\n");
    }else if (strstr(msg->topic, "/thing/property/set")){
        printf("📩 收到控制指令，开始解析...\n");

        //解析控制指令
        char *id = Onenet_Parse_Led(msg);
        if(id == NULL){
            fprintf(stderr, "failed to get id\n");
            free(id);
        }
        
        char payload[256];
        printf("%s: 📌 收到控制ID: %s\n",__FILE__, id);
        snprintf(payload, sizeof(payload), 
            "{\"id\":\"%s\",\"code\":200,\"msg\":\"success\"}",id);
        printf("reply_payload = %s\n",payload);
        onenet_publish_set_reply(client, payload);
        free(id);
    }
}

int main()
{
    onenet_client_t client;

    if (onenet_init(&client) != 0) {
        printf("Init failed\n");
        return -1;
    }

    if (onenet_connect(&client) != 0) {
        printf("Connection failed\n");
        return -1;
    }

    if (onenet_subscribe(&client, on_message) != 0) {
        printf("Subscribe failed\n");
        return -1;
    }

    onenet_loop_start(&client);

    for (int i = 0; i < 5; ++i) {
        char payload[256];
        snprintf(payload, sizeof(payload),
                 "{\"id\":\"%d\",\"params\":{\"temp\":{\"value\":%.1f}}}",
                 i, 25.0 + i);
        onenet_publish(&client, payload);
        sleep(5);
    }

    onenet_disconnect(&client);
    return 0;
}
