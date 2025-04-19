//system defined
#include <stdlib.h>
#include <pthread.h>
#include <wiringPi.h>
#include <signal.h>

//user defined
#include "msg_queue.h"
#include "control.h"
#include "global.h"
#include "voice_interface.h"
#include "socket_interface.h"
#include "smoke_interface.h"
#include "receive_interface.h"
#include "onenet_mqtt.h"

// main.c 顶部添加
ctrl_info_t *g_ctrl_info = NULL;
static pthread_t *g_tid = NULL;


static int detect_process(const char *process_name)     //判断进程是否在运行
{
    FILE *strm;
    char cmd[128] = {0};

    sprintf(cmd, "pgrep %s", process_name);     //使用pgrep指令可以直接获得进程的pid
    if ((strm = popen(cmd, "r")) == NULL)
    {
        return -1;
    }
    
    int pid;
    int ret = fscanf(strm, "%d", &pid);

    pclose(strm);

    return (ret == 1) ? pid : -1;
}


//释放链表节点
void release_control_list(struct control *phead)
{
    struct control *pointer = NULL;
    while(phead != NULL){
        pointer = phead;
        phead = phead->next;
        free(pointer);      //先将链表节点指向下一个后再释放当前这个节点
    }
}

void release_space(ctrl_info_t *ctrl_info, pthread_t *tid)
{
    if(ctrl_info != NULL){
        if(ctrl_info->ctrl_phead != NULL){
            release_control_list(ctrl_info->ctrl_phead);
        }
        free(ctrl_info);
    }

    if(tid != NULL)
        free(tid);
}

void signal_handler(int signum)
{
    printf("\nReceived signal %d (Ctrl+\\), cleaning up...\n", signum);

    if (g_ctrl_info && g_ctrl_info->mqtt_client) {
        onenet_disconnect(g_ctrl_info->mqtt_client);
    }

    if (g_ctrl_info) {
        if (g_ctrl_info->mqd != -1) {
            msg_queue_free(g_ctrl_info->mqd);
        }
    }

    release_space(g_ctrl_info, g_tid);

    exit(0);
}

int main(int argc, char **argv)
{
    signal(SIGQUIT, signal_handler);

    ctrl_info_t *ctrl_info = NULL;
    onenet_client_t client;
    struct control *pointer = NULL;
    int node_num = 0;
    int i;

    ctrl_info = malloc(sizeof(ctrl_info_t));
    if(ctrl_info == NULL){
        perror("malloc error!");
        return -1;
    }
    ctrl_info->mqd = -1;
    ctrl_info->ctrl_phead = NULL;
    ctrl_info->mqtt_client = NULL;

    if(wiringPiSetup() == -1){
        fprintf(stderr, "error: failed to load wiringPi lib\n");
        return -1;
    }

    /*查看mjpg_streamer服务是否在运行*/
    if(detect_process("mjpg") == -1){
        fprintf(stderr, "%s %s %d : failed to load orangepi camera process\n",__FILE__,__func__,__LINE__);
        exit(EXIT_FAILURE);
    }

    /* 初始化mqtt并连接到onenet */
    onenet_init(&client);
    onenet_connect(&client);
    printf("%s|%s|%d : connect onenet successfully\n",__FILE__,__func__,__LINE__);
    onenet_loop_start(&client);

    ctrl_info->mqd = msg_queue_create();
    if(ctrl_info->mqd == -1)
    {
        perror("ctrl_info->mqd");
        return -1;
    }
    //添加语音监听
    ctrl_info->ctrl_phead = add_VoiceInLink(ctrl_info->ctrl_phead);
    //添加socket监听
    ctrl_info->ctrl_phead = add_SocketInLink(ctrl_info->ctrl_phead);
    //添加烟雾监听
    ctrl_info->ctrl_phead = add_SmokeInLink(ctrl_info->ctrl_phead);
    ctrl_info->ctrl_phead = add_ReceiveInLink(ctrl_info->ctrl_phead);
    //添加火警监听
    //ctrl_info->ctrl_phead = add_FireInLink(ctrl_info->ctrl_phead);
    ctrl_info->mqtt_client = &client;
    g_ctrl_info = ctrl_info;

    pointer = ctrl_info->ctrl_phead;
    while(pointer != NULL){
        if(pointer->init != NULL){
            printf("%s|%s|%d control_name:%s\n",__FILE__,__func__,__LINE__,pointer->control_name);
            pointer->init();
        }
        node_num++;                 //统计链表节点并且执行初始化函数
        pointer = pointer->next;
    }

    //一次性开辟node_num个节点的线程标识符id
    pthread_t* tid = malloc(sizeof(pthread_t) * node_num);
    if(tid == NULL){
        perror("malloc error!");
        return -1;
    }
    g_tid = tid;

    pointer = ctrl_info->ctrl_phead;
    for(i = 0; i < node_num; i++){
        if(pointer->get != NULL){
            printf("%s|%s|%d control_name:%s\n",__FILE__,__func__,__LINE__,pointer->control_name);
            if(pthread_create(&tid[i], NULL, pointer->get, (void *)ctrl_info) != 0){
                perror("pthread_create");
                return -1;
            }
        }
        pointer = pointer->next;
    }

    for(i = 0; i < node_num; i++){
        pthread_join(tid[i], NULL);
    }

    pointer = ctrl_info->ctrl_phead;
    for(i = 0; i < node_num; i++){
        if(pointer->final != NULL){
            pointer->final();
        }
        pointer = pointer->next;
    }


    onenet_disconnect(&client);
    msg_queue_free(ctrl_info->mqd);
    release_space(ctrl_info, tid);
    return 0;
}