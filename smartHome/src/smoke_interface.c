#include "smoke_interface.h"
#include "receive_interface.h"
#include "gdevice.h"      // 用于 gdevice 链表操作
#include "ini.h"       // 如果你已有 ini 加载器
#include <string.h>

static struct gdevice *dev = NULL;
static struct gdevice *smoke_dev = NULL;

static int smoke_init(void)
{
    dev = get_device_list_head();
    smoke_dev = find_device_by_name(dev, "smoke");
    if (smoke_dev) {
        pinMode(smoke_dev->gpio_pin, smoke_dev->gpio_mode);
    } else {
        // fallback
        pinMode(SMOKE_PIN, SMOKE_MODE);
    }
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

    if (arg != NULL) {
        mqd = ((ctrl_info_t *)arg)->mqd;
    }
    if (mqd == (mqd_t)-1) {
        fprintf(stderr, "Error: Invalid mqd argument\n");
        pthread_exit(NULL);
    }

    printf("%s thread start\n", __func__);
    while (1) {
        // 支持从 gdevice 配置引脚
        int gpio_pin = smoke_dev ? smoke_dev->gpio_pin : SMOKE_PIN;

        status = digitalRead(gpio_pin);

        if (status == LOW && status_flag == 0) {
            buffer[2] = 0x45;
            buffer[3] = 0x00;

            printf("%s|%s|%d smoke alarm: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                   __FILE__, __func__, __LINE__, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

            if (mq_send(mqd, buffer, BUFFER_SIZE, 0) != -1) {
                status_flag = 1;
                if (smoke_dev) smoke_dev->gpio_status = LOW;
            }
        } else if (status == HIGH && status_flag == 1) {
            buffer[2] = 0x45;
            buffer[3] = 0x01;

            printf("%s|%s|%d alarm remove: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                   __FILE__, __func__, __LINE__, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

            if (mq_send(mqd, buffer, BUFFER_SIZE, 0) != -1) {
                status_flag = 0;
                if (smoke_dev) smoke_dev->gpio_status = HIGH;
            }
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
