#ifndef __GDEVICE_H__
#define __GDEVICE_H__

#include <stdio.h>
#include <wiringPi.h>

struct gdevice
{
    char dev_name[128]; //设备名称
    int key; //key值，用于匹配控制指令的值
    int gpio_pin; //控制的gpio引脚
    int gpio_mode; //输入输出模式
    int gpio_status; //高低电平状态
    int check_face_status; //是否进行人脸检测状态
    int check_voice_status; //是否语音语音播报
    int trigger_mode;  // 0: LOW触发，1: HIGH触发

    struct gdevice *next;
};

//extern struct gdevice *add_DeviceInLink(struct gdevice *device_head, struct gdevice *gdevice_interface);
extern struct gdevice *find_DeviceByKey(struct gdevice *device_head, int key);
extern int set_gpio_gedvice_status(struct gdevice *device);
#endif