#include "onenet_mosquitto.h"
#include "onenet_config.h"
#include "onenet_header.h"
#include "dht22_config.h"

onenet_client_t client;

void *sensor_publish_task(void *arg)
{
	assert(arg != NULL);
	
	onenet_client_t *client = (onenet_client_t *)arg;
	
	char buffer[40];
	char payload[512];
	int payloadlen = 0;
	int qos = 1;
	bool retain = false;
	bool led_status = false;

	//DHT22上电前延时2S以越过不稳定的状态后读取数据
	fprintf(stdout, "Waiting for DHT22 power-up....\n");
	delay_us_busy(2000000);

	while(1){	
		memset(buffer, '\0', sizeof(buffer));
		memset(payload, '\0', sizeof(payload));

		if(!DHT22_ReadData(&client->Data)){
			get_formatted_time(buffer, sizeof(buffer));
			led_status = digitalRead(LED_PIN) == HIGH ? true : false;
			fprintf(stdout, "time: %s\t temperature: %.2f °C\t humidity: %.2f%%RH\t led_status:%s\n",
							buffer, client->Data.temperature, client->Data.humidity, led_status ? "true" : "false");
			//构建消息体
			
			snprintf(payload, sizeof(payload), "{\"id\":\"123\",\"version\":\"1.0\",\
					\"params\":{\"led\":{\"value\":%s},\
					\"temp\":{\"value\":%.1f},\
					\"humi\":{\"value\":%.1f}}}", led_status ? "true" : "false", client->Data.temperature, client->Data.humidity);
			payloadlen = strlen(payload);		
			
			//发布消息
			client->ret = mosquitto_publish(client->mosq, NULL, client->topic.pub_prop_topic, payloadlen, payload, qos, retain);
			CHECK_MOSQ_ERR(client->ret, "❌消息发布失败,请重新尝试！！！", "✅消息发布成功", (void *)-1);
		}else{
			fprintf(stderr, "DHT22 read error\n");
		}
		delay_us_busy(2000000);
	}

	pthread_exit(NULL);
}

void signal_handler(int signum)
{
	if(signum == SIGINT){
		fprintf(stdout, "%s|%s|%d 收到SIGINT信号\n",__FILE__, __func__, __LINE__);
		onenet_disconnect(&client);
		exit(EXIT_SUCCESS);
	}
}

int main(void)
{
	int ret = -1;
	
	client.config.deviceID = "device01";
	client.config.productID = "z1z205B81V";
	client.config.token = "version=2018-10-31&res=products%2Fz1z205B81V%2Fdevices%2Fdevice01&et=1776075997&method=sha1&sign=vqDr0%2F2ILtpSTgDjqHgEnGdQCzY%3D";
	client.config.host = "mqtts.heclouds.com";
	client.config.port = 1883;
	client.config.keepalive = 120;
	
	
	if(signal(SIGINT, signal_handler) == SIG_ERR){
		fprintf(stderr, "%s|%s|%d 注册SIGINT信号失败\n", __FILE__, __func__, __LINE__);
		return -1;
	}
	
	if(wiringPiSetup() == -1){
		fprintf(stderr, "%s|%s|%d 初始化wiringPi库失败\n", __FILE__, __func__, __LINE__);
		return -1;
	}
	pinMode(LED_PIN, OUTPUT);
	
	if(onenet_init(&client) != 0){
		fprintf(stderr, "%s|%s|%d 初始化失败\n", __FILE__, __func__, __LINE__);
	}

	onenet_build_topics(&client);

	onenet_callback_set(&client);

	if(onenet_connect(&client) != 0){
		fprintf(stderr, "%s|%s|%d 服务器连接失败\n", __FILE__, __func__, __LINE__);
	}

	if(onenet_loop_start(&client) != 0){
		fprintf(stderr, "%s|%s|%d 启用后台线程失败\n", __FILE__, __func__, __LINE__);
	}
	fprintf(stdout, "%s|%s|%d 主线程ID: 0x%lx\n",__FILE__, __func__, __LINE__, pthread_self());
	
	pthread_t sensor_thread;

	ret = pthread_create(&sensor_thread, NULL, sensor_publish_task, (void *)&client);
	if(ret != 0){
		fprintf(stderr, "%s|%s|%d create thread error\n", __FILE__, __func__, __LINE__);
		return -1;
	}

	pthread_join(sensor_thread, NULL);

	while(1);

	onenet_disconnect(&client);

	return 0;
}
