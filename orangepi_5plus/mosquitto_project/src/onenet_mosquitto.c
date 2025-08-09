#include "onenet_config.h"
#include "onenet_header.h"
#include "onenet_parser.h"

int onenet_init(onenet_client_t *client)
{
	assert(client != NULL);

	//初始化mosquitto库
	client->ret = mosquitto_lib_init();
	CHECK_MOSQ_ERR(client->ret, "❌ 初始化mosquitto库失败", "✅ 初始化mosquitto库成功", -1);

	//创建新的客户端
	client->mosq = mosquitto_new(client->config.deviceID, true, client);
	if(client->mosq != NULL){
		fprintf(stdout, "%s|%s|%d ✅ 客户端创建成功\n",__FILE__,__func__,__LINE__);
	}else{
		fprintf(stderr, "%s|%s|%d ❌ 客户端创建失败\n",__FILE__,__func__,__LINE__);
		return -1;
	}

	//设置认证信息
	client->ret = mosquitto_username_pw_set(client->mosq, client->config.productID, client->config.token);
	CHECK_MOSQ_ERR(client->ret, "❌ 认证信息设置失败", "✅ 认证信息设置成功", -1);
	
	return client->ret;	
}

void onenet_build_topics(onenet_client_t *client)
{
	assert(client != NULL);

	snprintf(client->topic.pub_prop_topic, sizeof(client->topic.pub_prop_topic), 
			"$sys/%s/%s/thing/property/post",
			client->config.productID, client->config.deviceID);

	snprintf(client->topic.pub_prop_set_reply_topic, sizeof(client->topic.pub_prop_set_reply_topic), 
			"$sys/%s/%s/thing/property/set_reply",
			client->config.productID, client->config.deviceID);

	snprintf(client->topic.sub_prop_set_topic, sizeof(client->topic.sub_prop_set_topic), 
			"$sys/%s/%s/thing/property/set",
			client->config.productID, client->config.deviceID);

	snprintf(client->topic.sub_prop_reply_topic, sizeof(client->topic.sub_prop_reply_topic), 
			"$sys/%s/%s/thing/property/post/reply",
			client->config.productID, client->config.deviceID);

#if 0
	fprintf(stdout, "%s|%s|%d pub_prop_topic:%s\n", __FILE__, __func__, __LINE__, client->topic.pub_prop_topic);
	fprintf(stdout, "%s|%s|%d pub_prop_set_reply_topic:%s\n", __FILE__, __func__, __LINE__, client->topic.pub_prop_set_reply_topic);
	fprintf(stdout, "%s|%s|%d sub_prop_set_topic:%s\n", __FILE__, __func__, __LINE__, client->topic.sub_prop_set_topic);
	fprintf(stdout, "%s|%s|%d sub_prop_reply_topic:%s\n", __FILE__, __func__, __LINE__, client->topic.sub_prop_reply_topic);
#endif
}

int onenet_connect(onenet_client_t *client)
{
	assert(client != NULL);

	//连接服务器
	client->ret = mosquitto_connect(client->mosq, client->config.host, client->config.port, client->config.keepalive);
	CHECK_MOSQ_ERR(client->ret, "❌ 连接服务器失败", "✅ 连接服务器成功", -1);

	return client->ret;
}

int onenet_loop_start(onenet_client_t *client)
{
	assert(client != NULL);

	//启用后台线程处理事件
	client->ret = mosquitto_loop_start(client->mosq);
	CHECK_MOSQ_ERR(client->ret, "❌后台线程启用失败", "✅后台线程启用成功", -1);

	return client->ret;
}

int onenet_disconnect(onenet_client_t *client)
{
	//断开与服务器的连接
	client->ret = mosquitto_disconnect(client->mosq);
	CHECK_MOSQ_ERR(client->ret, "断开服务器连接失败", "成功断开与服务器的连接", -1);

	//停止后台线程
	client->ret = mosquitto_loop_stop(client->mosq, false);
	CHECK_MOSQ_ERR(client->ret, "❌停止后台线程失败", "✅成功停止后台线程", -1);

	//销毁客户端
	mosquitto_destroy(client->mosq);

	//释放资源
	mosquitto_lib_cleanup();

	return client->ret;
}

void on_connect(struct mosquitto *mosq, void *userdata, int rc)
{
	if(!rc){
		fprintf(stdout, "%s|%s|%d ✅ 连接服务器成功 后台线程ID: 0x%lx\n", __FILE__, __func__, __LINE__, pthread_self());
		
		//获取创建客户端时传进去的用户自定义指针
		assert(userdata != NULL);
		onenet_client_t *client = (onenet_client_t *)userdata;

		//订阅上报属性后的响应
		client->ret = mosquitto_subscribe(mosq, NULL, client->topic.sub_prop_reply_topic, 1);
		if(client->ret){
            fprintf(stderr, "%s|%s|%d ❌属性上报响应主题订阅失败\n", __FILE__, __func__, __LINE__);
        }else{
            fprintf(stdout, "%s|%s|%d ✅属性上报响应主题订阅成功\n", __FILE__, __func__, __LINE__);
        }
		//订阅属性设置指令
		client->ret = mosquitto_subscribe(mosq, NULL, client->topic.sub_prop_set_topic, 1);
		if(client->ret){
            fprintf(stderr, "%s|%s|%d ❌属性设置请求主题订阅失败\n", __FILE__, __func__, __LINE__);
        }else{
            fprintf(stdout, "%s|%s|%d ✅属性设置请求主题订阅成功\n", __FILE__, __func__, __LINE__);
        }
	}else{
		fprintf(stderr, "%s|%s|%d ❌ 连接服务器失败\n", __FILE__, __func__, __LINE__);
		return ;
	}
}

void on_subscribe(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
	printf("%s|%s|%d ✅客户端成功订阅,订阅数: %d 后台线程ID: 0x%lx\n", __FILE__, __func__, __LINE__, qos_count, pthread_self());
}

void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
	fprintf(stdout, "%s|%s|%d ✅ 消息发布成功\n", __FILE__, __func__, __LINE__);
}

void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	assert(userdata != NULL);

	const char *topic = message->topic;
    const char *payload = message->payload;
    const int payloadlen = message->payloadlen;
	onenet_client_t *client = (onenet_client_t *)userdata;
	

	fprintf(stdout, "%s|%s|%d 📩收到来自|%s|主题的消息|%s|消息长度|%d|\n",
				__FILE__, __func__, __LINE__, topic, payload, payloadlen);
	if(!strcmp(topic, client->topic.sub_prop_reply_topic)){
		fprintf(stdout, "%s|%s|%d 📩 收到属性上报的回复\n",__FILE__, __func__, __LINE__);
	}else if(!strcmp(topic, client->topic.sub_prop_set_topic)){
		fprintf(stdout, "%s|%s|%d 📩 收到属性设置请求，内容为: [%s]\n",
						__FILE__, __func__, __LINE__, payload);
		//回复控制指令和处理实际的物理设备
		handle_control_message(payload, client);
	}else{
		fprintf(stdout, "%s|%s|%d ⚠  收到未知主题消息: [%s]\n", 
							__FILE__, __func__, __LINE__, topic);
	}
}

void on_disconnect(struct mosquitto *mosq, void *userdata, int rc)
{
	if(!rc){
		fprintf(stdout, "%s|%s|%d ✅ 正常断开客户端与服务器的连接\n", __FILE__, __func__, __LINE__);
	}else{
		fprintf(stderr, "%s|%s|%d ❌ 异常断开客户端与服务器的连接\n", __FILE__, __func__, __LINE__);
		return;
	}
}

void onenet_callback_set(onenet_client_t *client)
{
	assert(client != NULL);

	mosquitto_connect_callback_set(client->mosq, on_connect);
	mosquitto_subscribe_callback_set(client->mosq, on_subscribe);	
	mosquitto_publish_callback_set(client->mosq, on_publish);
	mosquitto_message_callback_set(client->mosq, on_message);
	mosquitto_disconnect_callback_set(client->mosq, on_disconnect);
}


