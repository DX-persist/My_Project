#include "smoke_interface.h"

static int smoke_init(void)
{
    pinMode(SMOKE_PIN, SMOKE_MODE);

    return 0;
}

static void smoke_final(void)
{
    //do nothing
}

static void* smoke_get(void *arg)
{
    pthread_detach(pthread_self());
    int status = HIGH;
    int status_flag = 0;
    unsigned char buffer[BUFFER_SIZE] = {0xAA, 0x55, 0x00, 0x00, 0x55, 0xAA};
    mqd_t mqd = -1;
    int length = -1;

    if(arg != NULL){
        mqd = ((ctrl_info_t *)arg)->mqd;
    }
    if(mqd == (mqd_t)-1){
        fprintf(stderr, "Error: Invalid mqd argument\n");
        pthread_exit(NULL);
    }

    printf("%s thread start\n",__func__);
    while(1)
    {
        status = digitalRead(SMOKE_PIN);
        if(status == LOW && status_flag == 0)
        {
            buffer[2] = 0x45;
            buffer[3] = 0x00;       //如果蜂鸣器是低电平触发，需要修改
            printf("%s|%s|%d smoke alarm: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",__FILE__, __func__, 
                __LINE__, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
            length = mq_send(mqd, buffer, BUFFER_SIZE, 0);
            if(length == -1)
                continue;
            status_flag = 1;
        }
        else if(status == HIGH && status_flag == 1)
        {
            buffer[2] = 0x45;
            buffer[3] = 0x01;
            printf("%s|%s|%d alarm remove: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",__FILE__, __func__, 
                __LINE__, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
            length = mq_send(mqd, buffer, BUFFER_SIZE, 0);
            if(length == -1)
                continue;
            status_flag = 0;
        }

        sleep(5);
    }
    pthread_exit(NULL);
}
#if 0
static void* smoke_set(void *arg)
{

}
#endif

struct control smoke_control = {
    .control_name = "smoke",
    .init = smoke_init,
    .final = smoke_final,
    .get = smoke_get,
    .set = NULL,
    
    .next = NULL
};

struct control *add_SmokeInLink(struct control *control_head)
{
    return add_InterfaceInLink(control_head, &smoke_control);
}