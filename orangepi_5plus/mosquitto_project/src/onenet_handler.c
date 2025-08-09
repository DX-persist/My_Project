#include "onenet_header.h"
#include "onenet_parser.h"
#include "onenet_config.h"

static int parse_control_command(const char *json_payload, device_command_t *cmd)
{
	cJSON *root = cJSON_Parse(json_payload);
	if(root == NULL){
		fprintf(stderr, "%s|%s|%d 消息为空,无法解析\n",__FILE__, __func__, __LINE__);
		return -1;
	}

	cJSON *id = cJSON_GetObjectItem(root, "id");
	if(id != NULL && id->type == cJSON_String){
	 	strncpy(cmd->id, id->valuestring, sizeof(cmd->id) - 1);
	}else{
		fprintf(stderr, "%s|%s|%d id字段无效或者不存在\n", __FILE__, __func__, __LINE__);
		return -1;
	}
	
	cJSON *params = cJSON_GetObjectItem(root, "params");
	if(params){
		cJSON *led_status = cJSON_GetObjectItem(params, "led");
		if(led_status && cJSON_IsBool(led_status)){
			strncpy(cmd->led_cmd, cJSON_IsTrue(led_status) ? "true" : "false", sizeof(cmd->led_cmd) - 1);
		}else{
			fprintf(stderr, "%s|%d ⚠ led字段类型不是bool\n",__func__,__LINE__);
		}
	}

	cJSON_Delete(root);

	return 0;
}

static void reply_to_server(const char *id, const char *result, onenet_client_t *client)
{
	char buffer[256];

	memset(buffer, '\0', sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "{\"id\":\"%s\",\"code\":200,\"msg\":\"%s\"}", id, result);
									 
	mosquitto_publish(client->mosq, NULL, 
			client->topic.pub_prop_set_reply_topic, 
			strlen(buffer), buffer, 1, false);
}

static void control_device(const device_command_t *cmd)
{
	if(strcmp(cmd->led_cmd, "true") == 0){
    	digitalWrite(LED_PIN, HIGH);
	}else{
		digitalWrite(LED_PIN, LOW);
	}
}

static void report_device_status(onenet_client_t *client, device_command_t *cmd)
{
	if(digitalRead(LED_PIN) == HIGH){
		cmd->led_status = true;
	}else{
		cmd->led_status = false;
	}

	char report_json[256];
	memset(report_json ,'\0', sizeof(report_json));
	snprintf(report_json, sizeof(report_json), "{\"id\":\"123\",\"version\":\"1.0\",\
						\"params\":{\"led\":{\"value\":%s},\
						\"temp\":{\"value\":%.1f},\
						\"humi\":{\"value\":%.1f}}}", (cmd->led_status == true) ? "true" : "false",
						client->Data.temperature, client->Data.humidity);

	mosquitto_publish(client->mosq, NULL, client->topic.pub_prop_topic,
						strlen(report_json), report_json, 1, false);
}

void handle_control_message(const char *payload, onenet_client_t *client)
{
	device_command_t cmd = {0};
	
	if(parse_control_command(payload, &cmd) != 0){
		fprintf(stderr, "%s|%s|%d 消息为空,无法解析\n",__FILE__, __func__, __LINE__);
	}

	reply_to_server(cmd.id, "执行成功", client);

	control_device(&cmd);

	report_device_status(client, &cmd);
}