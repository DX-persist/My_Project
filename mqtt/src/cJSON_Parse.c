#include <cjson/cJSON.h>
#include <stdio.h>
#include <mosquitto.h>
#include <string.h>
#include <stdlib.h>

char *Onenet_Parse_Led(const struct mosquitto_message *msg)
{
    char *id_value = NULL;
    
    if(msg == NULL || msg->payload == NULL){
        fprintf(stderr, "❌ 消息为空，无法解析\n");
        return NULL;
    }

    printf("🔍 Payload: %s\n",(char *)msg->payload);

	cJSON *root = cJSON_Parse((char *)msg->payload);
    if(root == NULL){
        fprintf(stderr, "⚠ 无法解析JSON\n");
        return NULL;
    }

    cJSON *id = cJSON_GetObjectItem(root, "id");
    if(id != NULL && id->type == cJSON_String){
        id_value = strdup(id->valuestring);         //使用strdup函数复制原字符串的内容
    }else{
        fprintf(stderr, "⚠ id 字段无效或不存在");
        return NULL;
    }
    
    cJSON *params = cJSON_GetObjectItem(root, "params");
    if(params == NULL){
        fprintf(stderr, "⚠ params 字段无效或不存在\n");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON *led = cJSON_GetObjectItem(params, "led");   
    if(led == NULL){
        fprintf(stderr, "⚠ led 字段无效或不存在\n");
        cJSON_Delete(root);
        return NULL;
    } 
      
    if(cJSON_IsTrue(led)){
        printf("led 打开🗝\n");
    } else if(cJSON_IsFalse(led)){
        printf("led 关闭🔒\n");
    } 

    cJSON_Delete(root);

    return id_value;
}
