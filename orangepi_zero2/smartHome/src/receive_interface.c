#include "receive_interface.h"

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#define LOW_TRIGGER  0
#define HIGH_TRIGGER 1

typedef struct {
    int msg_length;
    unsigned char *buffer;
    ctrl_info_t *ctrl_info;
} recv_msg_t;

static struct gdevice *pdevhead = NULL;

static const char* get_device_action_state(struct gdevice *dev) {
    if (!dev) return "Unknown";
    return (dev->trigger_mode == HIGH_TRIGGER) ?
           (dev->gpio_status == HIGH ? "Open" : "Close") :
           (dev->gpio_status == LOW ? "Open" : "Close");
}

static int handler_gdevice(void* user, const char* section, const char* name, const char* value) {
    struct gdevice *pdevtemp = NULL;

    if (!pdevhead) {
        pdevhead = calloc(1, sizeof(struct gdevice));
        strcpy(pdevhead->dev_name, section);
    } else if (strcmp(pdevhead->dev_name, section) != 0) {
        pdevtemp = calloc(1, sizeof(struct gdevice));
        strcpy(pdevtemp->dev_name, section);
        pdevtemp->next = pdevhead;
        pdevhead = pdevtemp;
    }

    if (MATCH(pdevhead->dev_name, "key")) sscanf(value, "%x", &pdevhead->key);
    else if (MATCH(pdevhead->dev_name, "gpio_pin")) pdevhead->gpio_pin = atoi(value);
    else if (MATCH(pdevhead->dev_name, "gpio_mode")) pdevhead->gpio_mode = !strcmp(value, "OUTPUT") ? OUTPUT : INPUT;
    else if (MATCH(pdevhead->dev_name, "gpio_status")) pdevhead->gpio_status = !strcmp(value, "HIGH") ? HIGH : LOW;
    else if (MATCH(pdevhead->dev_name, "check_face_status")) pdevhead->check_face_status = atoi(value);
    else if (MATCH(pdevhead->dev_name, "check_voice_status")) pdevhead->check_voice_status = atoi(value);
    else if (MATCH(pdevhead->dev_name, "trigger_mode")) pdevhead->trigger_mode = !strcmp(value, "HIGH") ? HIGH_TRIGGER : LOW_TRIGGER;

    return 1;
}

static int receive_init(void) {
    if (ini_parse("/etc/gdevice.ini", handler_gdevice, NULL) < 0) {
        printf("Can't load 'gdevice.ini'\n");
        return 1;
    }
    IO_Init();
    OLED_Init();
    OLED_ColorTurn(0);
    OLED_DisplayTurn(0);
    OLED_Clear();
    face_Init();

    struct gdevice *dev = pdevhead;
    while (dev) {
        set_gpio_gedvice_status(dev);
        dev = dev->next;
    }

    return 0;
}

static void receive_final(void) {
    face_Final();
}

static void* handler_device(void *arg) {
    recv_msg_t *recv_msg = (recv_msg_t *)arg;
    if (!recv_msg || !recv_msg->buffer || recv_msg->msg_length < 6) pthread_exit(NULL);
    if (recv_msg->buffer[2] == 0x40) pthread_exit(NULL);

    struct gdevice *dev = find_DeviceByKey(pdevhead, recv_msg->buffer[2]);
    if (!dev) pthread_exit(NULL);

    dev->gpio_status = (dev->trigger_mode == HIGH_TRIGGER) ?
                       (recv_msg->buffer[3] == 0 ? HIGH : LOW) :
                       (recv_msg->buffer[3] == 0 ? LOW : HIGH);

    int ret = -1;
    double face_result = 0.0;
    if (dev->check_face_status) {
        face_result = face_identification();
        if (face_result >= 0.6) {
            recv_msg->buffer[2] = 0x47;
            ret = set_gpio_gedvice_status(dev);
        } else {
            recv_msg->buffer[2] = 0x46;
            pthread_exit(NULL);
        }
    } else {
        ret = set_gpio_gedvice_status(dev);
    }

    char oled_msg[256];
    bool is_alert = false;

    // 🚨 烟雾/可燃气体传感器报警处理
    if (recv_msg->buffer[2] == 0x45 && recv_msg->buffer[3] == 0) {
        OLED_Clear();
        OLED_ShowString(0, 0, "WARNING: Gas/Fire!", 8);
        printf("ALERT: Smoke sensor triggered!\n");
        is_alert = true;
    }

    // ✅ 正常设备状态显示（非警告状态才显示）
    if (!is_alert) {
        snprintf(oled_msg, sizeof(oled_msg), "%s %s %s!",
                get_device_action_state(dev), dev->dev_name, ret == -1 ? "failed" : "success");
        OLED_Clear();
        OLED_ShowString(0, 0, oled_msg, 8);
        printf("%s : oled_msg:%s\n", __func__, oled_msg);
}


    if (dev->check_voice_status && recv_msg->ctrl_info) {
        struct control *ctrl = recv_msg->ctrl_info->ctrl_phead;
        while (ctrl) {
            if (strstr(ctrl->control_name, "voice")) {
                pthread_t tid;
                pthread_create(&tid, NULL, ctrl->set, (void *)recv_msg->buffer);
                break;
            }
            ctrl = ctrl->next;
        }
    }

    if (dev->check_face_status && face_result >= 0.6) {
        sleep(5);
        dev->gpio_status = dev->trigger_mode == HIGH_TRIGGER ? LOW : HIGH;
        set_gpio_gedvice_status(dev);
        snprintf(oled_msg, sizeof(oled_msg), "%s %s success!", get_device_action_state(dev), dev->dev_name);
        OLED_Clear();
        printf("%s : oled_msg:%s\n",__func__,oled_msg);
        OLED_ShowString(0, 0, oled_msg, 8);
    }

    pthread_exit(NULL);
}

static void* receive_get(void *arg) {
    if (!arg) pthread_exit(NULL);

    ctrl_info_t *ctrl_info = (ctrl_info_t *)arg;
    struct mq_attr attr;
    if (mq_getattr(ctrl_info->mqd, &attr) == -1) pthread_exit(NULL);

    recv_msg_t *recv_msg = calloc(1, sizeof(recv_msg_t));
    recv_msg->ctrl_info = ctrl_info;
    recv_msg->buffer = calloc(1, attr.mq_msgsize);

    char *buffer = calloc(1, attr.mq_msgsize);
    pthread_detach(pthread_self());

    while (1) {
        ssize_t len = mq_receive(ctrl_info->mqd, buffer, attr.mq_msgsize, NULL);
        if (len > 0 && buffer[0] == 0xAA && buffer[1] == 0x55 && buffer[4] == 0x55 && buffer[5] == 0xAA) {
            recv_msg->msg_length = len;
            memcpy(recv_msg->buffer, buffer, len);
            pthread_t tid;
            pthread_create(&tid, NULL, handler_device, (void *)recv_msg);
        }
    }

    pthread_exit(NULL);
}

struct control receive_control = {
    .control_name = "receive",
    .init = receive_init,
    .final = receive_final,
    .get = receive_get,
    .set = NULL,
    .next = NULL
};

struct control *add_ReceiveInLink(struct control *control_head) {
    return add_InterfaceInLink(control_head, &receive_control);
}