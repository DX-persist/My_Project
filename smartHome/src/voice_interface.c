#include "voice_interface.h"

#if 0

struct control
{
    char control_name[128];
    int (*init)(void);
    void (*final)(void);
    void *(*get)(void *arg);
    void *(*set)(void *arg);
    
    struct control *next;
};

#endif

static int serial_fd = -1;

static int voice_init(void)
{
    serial_fd = serialOpen (SERIAL_DEV, BAUD);
    printf("%s|%s|%d :serial_fd = %d\n",__FILE__, __func__, __LINE__, serial_fd);

    return serial_fd;
}

static void voice_final(void)
{
    if(serial_fd != -1){                //关闭串口
        serialClose(serial_fd);
        serial_fd = -1;
    }
}

static void *voice_get(void *arg)
{
    int receive_length = 0;
    unsigned char buffer[CMD_BUFFER_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  //数据帧格式
    mqd_t mqd = -1;
    
	if(serial_fd == SERIAL_INIT_FAILURE)
    {
        serial_fd = voice_init();
        if(serial_fd == SERIAL_INIT_FAILURE)
        {
            printf("%s|%s|%d failed to open serial\n",__FILE__,__func__,__LINE__);
            pthread_exit(NULL);
        }   
    }
    
    if(arg != NULL){
        mqd = ((ctrl_info_t *)arg)->mqd;
    }

    if(mqd == (mqd_t)-1)
        pthread_exit(NULL);

    pthread_detach(pthread_self());
    printf("%s thread start\n",__func__);

    while(1)
    {
    	receive_length = serialGetstring(serial_fd,buffer); 
        printf("%s|%s|%d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",__FILE__, __func__, 
                __LINE__, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
        printf("%s|%s|%d : receive_length = %d\n",__FILE__, __func__, __LINE__, receive_length);
        if(receive_length > 0)
        {
            if(buffer[0] == 0xAA && buffer[1] == 0x55 
                && buffer[4] == 0x55 &&buffer[5] == 0xAA){
                printf("%s|%s|%d send: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",__FILE__, __func__, 
                    __LINE__, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
                msg_queue_send(mqd, buffer, receive_length);
            }
            memset(buffer, 0, sizeof(buffer));
        }
    }

    pthread_exit(NULL);
}

static void *voice_set(void *arg)
{
    unsigned char *buffer = (unsigned char *)arg;
    int transmit_length = 0;
    size_t min_length = 6;  // 预期的 buffer 最小长度

    // 检查 buffer 是否有效
    if (buffer == NULL) {
        fprintf(stderr, "Error: buffer is NULL in %s\n", __func__);
        pthread_exit(NULL);
    }

    // 检查 serial_fd 是否有效
    if (serial_fd == SERIAL_INIT_FAILURE)
    {
        serial_fd = voice_init();
        if (serial_fd == SERIAL_INIT_FAILURE)
        {
            fprintf(stderr, "Error: Failed to open serial in %s\n", __func__);
            pthread_exit(NULL);
        }
    }

    // 打印消息前，确保 buffer 至少有 6 个字节
    if (sizeof(buffer) < min_length) {
        fprintf(stderr, "Error: buffer is too short in %s\n", __func__);
        pthread_exit(NULL);
    }

    printf("%s|%s|%d send message to queue: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
           __FILE__, __func__, __LINE__, 
           buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

    // 发送数据
    transmit_length = serialSendstring(serial_fd, buffer, min_length);
    if (transmit_length > 0)
    {
        printf("%s: successfully transmitted message to the voice module\n", __func__);
    }
    else
    {
        fprintf(stderr, "Error: Failed to transmit message\n");
    }

    pthread_exit(NULL);
}


struct control voice_control = {
    .control_name = "voice",
    .init = voice_init,
    .final = voice_final,
    .get = voice_get,
    .set = voice_set,

    .next = NULL
};

struct control *add_VoiceInLink(struct control *control_head)
{
    return add_InterfaceInLink(control_head, &voice_control);
}