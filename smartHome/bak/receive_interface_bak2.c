#include "receive_interface.h"

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#define LOW_TRIGGER  0
#define HIGH_TRIGGER 1

typedef struct
{
    int msg_length;
    unsigned char *buffer;
    ctrl_info_t *ctrl_info;
}recv_msg_t;

// 新增函数：根据 trigger_mode 和 gpio_status 解析设备当前是否处于“打开”状态
static const char* get_device_action_state(struct gdevice *dev) {
    if (dev == NULL) return "Unknown";

    if (dev->trigger_mode == HIGH_TRIGGER) {
        return (dev->gpio_status == HIGH) ? "Open" : "Close";
    } else { // LOW_TRIGGER
        return (dev->gpio_status == LOW) ? "Open" : "Close";
    }
}

static struct gdevice *pdevhead = NULL;

static int handler_gdevice(void* user, const char* section, const char* name,const char* value)
{
	struct gdevice *pdevtemp = NULL;

	if(pdevhead == NULL)
	{
		pdevhead = (struct gdevice *)malloc(sizeof(struct gdevice));
		pdevhead->next = NULL;
		memset(pdevhead, 0, sizeof(struct gdevice));
		strcpy(pdevhead->dev_name, section);
	}
	else if(strcmp(pdevhead->dev_name, section) != 0)
	{
		pdevtemp = (struct gdevice *)malloc(sizeof(struct gdevice));
		memset(pdevtemp, 0, sizeof(struct gdevice));
		strcpy(pdevtemp->dev_name, section);
		pdevtemp->next = pdevhead;
		pdevhead = pdevtemp;
	}

	if(pdevhead != NULL)
	{
		if(MATCH(pdevhead->dev_name, "key"))	
		{
			sscanf(value, "%x", &pdevhead->key);
			//printf("%d 0x%x\n",__LINE__,pdevhead->key);
		}
		else if((MATCH(pdevhead->dev_name, "gpio_pin")))
		{
			pdevhead->gpio_pin = atoi(value);
		}
		else if((MATCH(pdevhead->dev_name, "gpio_mode")))
		{
			if(!strcmp(value, "OUTPUT"))
			{
				pdevhead->gpio_mode = OUTPUT;
			}
			else if(!(value, "INPUT"))
			{
				pdevhead->gpio_mode = INPUT;
			}
		}
		else if((MATCH(pdevhead->dev_name, "gpio_status")))
		{
			if(!strcmp(value, "HIGH"))
			{
				pdevhead->gpio_status = HIGH;
			}
			else if(!strcmp(value, "LOW"))
			{
				pdevhead->gpio_status = LOW;
			}
		}
		else if((MATCH(pdevhead->dev_name, "check_face_status")))
		{
			pdevhead->check_face_status = atoi(value);
		}
		else if((MATCH(pdevhead->dev_name, "check_voice_status")))
		{
			pdevhead->check_voice_status = atoi(value);
		}
        else if (MATCH(pdevhead->dev_name, "trigger_mode"))
        {
            if (!strcmp(value, "HIGH")) {
                pdevhead->trigger_mode = 1;
            } else {
                pdevhead->trigger_mode = 0;
            }
        }

	}
	//printf("section:%s\tname:%s\tvalue:%s\n",section,name,value);
	return 1;
}

static int receive_init(void)
{
    if(ini_parse("/etc/gdevice.ini", handler_gdevice, NULL) < 0) 
    {
        printf("Can't load 'gdevice.ini'\n");
        return 1;
    }

    //初始化OLED
    IO_Init();
    OLED_Init();
    OLED_ColorTurn(0);//0正常显示，1 反色显示
    OLED_DisplayTurn(0);//0正常显示 1 屏幕翻转显示
    OLED_Clear();

    //初始化人脸识别接口
    face_Init();
#if 0
    struct gdevice *pdevtemp = pdevhead;
	while(pdevtemp)
	{
		printf("pdev->name:%s\n",pdevtemp->dev_name);
		printf("pdev->key:%x\n",pdevtemp->key);
		printf("pdev->gpio_pin:%d\n",pdevtemp->gpio_pin);
		printf("pdev->gpio_mode:%d\n",pdevtemp->gpio_mode);
		printf("pdev->gpio_status:%d\n",pdevtemp->gpio_status);
		printf("pdev->check_face_status:%d\n",pdevtemp->check_face_status);
		printf("pdev->check_voice_status:%d\n",pdevtemp->check_voice_status);

		pdevtemp = pdevtemp->next;
	}
#endif
    return 0;
}

static void receive_final(void)
{
    face_Final();
}

#if 0
static void* handler_device(void *arg)
{
    recv_msg_t *recv_msg = NULL;
    struct gdevice *current_pdevice = NULL;
    char success_or_failed[20] = "success";
    int ret = -1;
    pthread_t tid = -1;
    int smoke_status = 0;
    double face_result = 0.0;

    pthread_detach(pthread_self());
    printf("%s thread started\n", __func__);

    if (arg == NULL) {
        fprintf(stderr, "Error: arg is NULL in %s\n", __func__);
        pthread_exit(NULL);
    }
    recv_msg = (recv_msg_t *)arg;

    if (recv_msg->buffer == NULL) {
        fprintf(stderr, "Error: recv_msg->buffer is NULL in %s\n", __func__);
        pthread_exit(NULL);
    }

    if (recv_msg->msg_length < 6) {
        fprintf(stderr, "Error: recv_msg->msg_length (%d) is too small in %s\n", recv_msg->msg_length, __func__);
        pthread_exit(NULL);
    }

    printf("%s: handler_thread:::recv_msg->msg_length = %d\n", __func__, recv_msg->msg_length);
    printf("%s|%s|%d: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __FILE__, __func__, __LINE__,
           recv_msg->buffer[0], recv_msg->buffer[1], recv_msg->buffer[2], 
           recv_msg->buffer[3], recv_msg->buffer[4], recv_msg->buffer[5]);

    // **临时修复: 如果 buffer[2] == 0x40，忽略处理，防止找不到设备**
    if (recv_msg->buffer[2] == 0x40) {
        printf("Info: Ignoring wake-up command (0x40)\n");
        pthread_exit(NULL);
    }

    if (pdevhead == NULL) {
        fprintf(stderr, "Error: pdevhead is NULL in %s\n", __func__);
        pthread_exit(NULL);
    }

    current_pdevice = find_DeviceByKey(pdevhead, recv_msg->buffer[2]);
    if (current_pdevice == NULL) {
        fprintf(stderr, "Error: Device not found for key 0x%x\n", recv_msg->buffer[2]);
        pthread_exit(NULL);
    }

    current_pdevice->gpio_status = recv_msg->buffer[3] == 0 ? HIGH : LOW;
    if(current_pdevice->check_face_status == 1){
        face_result = face_identification();
        printf("%s: face_ret = %.2lf\n",__func__,face_result);

        if(face_result >= 0.6){
            printf("Info: Face recognition success, face_result = %.2f\n", face_result);
            ret = set_gpio_gedvice_status(current_pdevice);
            recv_msg->buffer[2] = 0x47;   //人脸识别成功播报
        }else{
            printf("Warning: Face recognition failed, face_result = %.2f\n", face_result);
            recv_msg->buffer[2] = 0x46;   //人脸识别失败播报
            ret = -1;
            pthread_exit(NULL);
        }
    }else if (current_pdevice->check_face_status == 0){
        ret = set_gpio_gedvice_status(current_pdevice);
    }
    

    if (ret == -1) {
        memset(success_or_failed, '\0', sizeof(success_or_failed));
        strncpy(success_or_failed, "failed", 6);
    }

    if (current_pdevice->check_voice_status == 1) {
        if (recv_msg->ctrl_info == NULL) {
            fprintf(stderr, "Error: recv_msg->ctrl_info is NULL in %s\n", __func__);
            pthread_exit(NULL);
        }
        struct control *pcontrol = recv_msg->ctrl_info->ctrl_phead;
        while (pcontrol != NULL) {
            if (strstr(pcontrol->control_name, "voice")){
                if(recv_msg->buffer[2] == 0x45 && recv_msg->buffer[3] == 0){
                     smoke_status = 1;
                }
                pthread_create(&tid, NULL, pcontrol->set, (void *)recv_msg->buffer);
                break;
            } 
            pcontrol = pcontrol->next;
        }
    }

    char oled_msg[512];
    memset(oled_msg, '\0', sizeof(oled_msg));
    snprintf(oled_msg, sizeof(oled_msg), "%s %s %s!", current_pdevice->gpio_status == LOW ? "Open" : "Close", 
                current_pdevice->dev_name, success_or_failed);
    if(smoke_status == 1){
        memset(oled_msg, '\0', sizeof(oled_msg));
        strcpy(oled_msg, "WARNING: Gas/Fire!");
    }
    printf("%s|%s|%d: %s",__FILE__,__func__,__LINE__,oled_msg);
    OLED_Clear();
    OLED_ShowString(0, 0, oled_msg, 8);

    if(current_pdevice->check_face_status == 1 && ret == 0 && face_result >= 0.6)
    {
        printf(">>> Face success, sleeping 5s then closing...\n");
        sleep(5);
        current_pdevice->gpio_status = LOW;
        printf(">>> Closing device %s (gpio = %d)\n", current_pdevice->dev_name, current_pdevice->gpio_status);
        ret = set_gpio_gedvice_status(current_pdevice);
        printf(">>> Setting pin %d to %s\n", current_pdevice->gpio_pin, current_pdevice->gpio_status == LOW ? "LOW" : "HIGH");

        //set_gpio_gedvice_status(current_pdevice);
         // **确保 OLED 更新**
        snprintf(oled_msg, sizeof(oled_msg), "%s %s %s!", 
        current_pdevice->gpio_status == LOW ? "Open" : "Close", 
        current_pdevice->dev_name, success_or_failed);
        OLED_Clear();
        OLED_ShowString(0, 0, oled_msg, 8);
    }

    pthread_exit(NULL);
}
#endif

static void* handler_device(void *arg)
{
    recv_msg_t *recv_msg = NULL;
    struct gdevice *current_pdevice = NULL;
    char success_or_failed[20] = "success";
    int ret = -1;
    pthread_t tid = -1;
    int smoke_status = 0;
    double face_result = 0.0;

    pthread_detach(pthread_self());

    if (arg == NULL || ((recv_msg = (recv_msg_t *)arg)->buffer == NULL) || recv_msg->msg_length < 6) {
        pthread_exit(NULL);
    }

    if (recv_msg->buffer[2] == 0x40) {
        pthread_exit(NULL);
    }

    if ((current_pdevice = find_DeviceByKey(pdevhead, recv_msg->buffer[2])) == NULL) {
        pthread_exit(NULL);
    }

    current_pdevice->gpio_status =
        current_pdevice->trigger_mode == 1 ?
        (recv_msg->buffer[3] == 0 ? HIGH : LOW) :
        (recv_msg->buffer[3] == 0 ? LOW : HIGH);

    if(current_pdevice->check_face_status == 1){
        face_result = face_identification();
        if(face_result >= 0.6){
            ret = set_gpio_gedvice_status(current_pdevice);
            recv_msg->buffer[2] = 0x47;
        }else{
            recv_msg->buffer[2] = 0x46;
            pthread_exit(NULL);
        }
    }else{
        ret = set_gpio_gedvice_status(current_pdevice);
    }

    if (ret == -1) {
        strncpy(success_or_failed, "failed", sizeof(success_or_failed));
    }

    if (current_pdevice->check_voice_status == 1 && recv_msg->ctrl_info) {
        struct control *pcontrol = recv_msg->ctrl_info->ctrl_phead;
        while (pcontrol) {
            if (strstr(pcontrol->control_name, "voice")) {
                if(recv_msg->buffer[2] == 0x45 && recv_msg->buffer[3] == 0){
                    smoke_status = 1;
                }
                pthread_create(&tid, NULL, pcontrol->set, (void *)recv_msg->buffer);
                break;
            }
            pcontrol = pcontrol->next;
        }
    }

    char oled_msg[512] = {0};
    snprintf(oled_msg, sizeof(oled_msg), "%s %s %s!",
         get_device_action_state(current_pdevice),
         current_pdevice->dev_name, success_or_failed);


    if(smoke_status == 1){
        strcpy(oled_msg, "WARNING: Gas/Fire!");
    }

    OLED_Clear();
    OLED_ShowString(0, 0, oled_msg, 8);

    if(current_pdevice->check_face_status == 1 && ret == 0 && face_result >= 0.6)
    {
        sleep(5);
        current_pdevice->gpio_status = current_pdevice->trigger_mode == 1 ? LOW : HIGH;
        ret = set_gpio_gedvice_status(current_pdevice);

        snprintf(oled_msg, sizeof(oled_msg), "%s %s %s!", 
         get_device_action_state(current_pdevice),
         current_pdevice->dev_name, success_or_failed);

        OLED_Clear();
        OLED_ShowString(0, 0, oled_msg, 8);
    }

    pthread_exit(NULL);
}

static void *receive_get(void *arg)
{
    struct mq_attr attr;
    ssize_t recv_length = -1;
    char *buffer = NULL;
    pthread_t tid;
    recv_msg_t *recv_msg = NULL;

    if(arg != NULL){
        recv_msg = (recv_msg_t *)malloc(sizeof(recv_msg_t));
        recv_msg->ctrl_info = (ctrl_info_t *)arg;
        recv_msg->msg_length = -1;
        recv_msg->buffer = NULL;    
    }else{
        fprintf(stderr, "Error: Invalid arg argument\n");
        pthread_exit(NULL);
    }

    if(mq_getattr(recv_msg->ctrl_info->mqd, &attr) == -1){
        perror("mq_getattr");
        pthread_exit(NULL);
    }

    recv_msg->buffer = (unsigned char *)malloc(sizeof(attr.mq_msgsize));
    buffer = (char *)malloc(sizeof(attr.mq_msgsize));
    memset(recv_msg->buffer, 0, sizeof(attr.mq_msgsize));
    memset(buffer, 0, sizeof(attr.mq_msgsize));

    pthread_detach(pthread_self());
    printf("%s thread started\n", __func__);
    while(1)
    {
        recv_length = mq_receive(recv_msg->ctrl_info->mqd, buffer, attr.mq_msgsize, NULL);
        printf("%s|%s|%d: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",__FILE__, __func__, 
                __LINE__, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
        printf("%s|%s|%d recv_length:%ld\n",__FILE__, __func__, __LINE__,recv_length);
        if(recv_length == -1){
            if(errno == EAGAIN){
                fprintf(stderr, "queue is empty\n");
            }else{
                perror("mq_receive");
            }
        }else if(recv_length > 0){
            if(buffer[0] == 0xAA && buffer[1] == 0x55 
                && buffer[4] == 0x55 &&buffer[5] == 0xAA){
                    recv_msg->msg_length = recv_length;
                    memcpy(recv_msg->buffer, buffer, recv_msg->msg_length);
                    printf("%s: preparing to execute handler_device\n",__func__);
                    pthread_create(&tid, NULL, handler_device, (void *)recv_msg);
            }
        }
    }
 
    pthread_exit(NULL);
}

#if 0
static void *receive_set(void *arg)
{

}
#endif

struct control receive_control = {
    .control_name = "receive",
    .init = receive_init,
    .final = receive_final,
    .get = receive_get,
    .set = NULL,
//   .set = receive_set,
   .next = NULL
};

struct control *add_ReceiveInLink(struct control *control_head)
{
    return add_InterfaceInLink(control_head, &receive_control);
}