#include "gdevice.h"

#if 0
struct gdevice *add_DeviceInLink(struct gdevice *device_head, struct gdevice *gdevice_interface)
{
    if(device_head == NULL)
    {
        device_head = gdevice_interface;
    }
    else
    {
        gdevice_interface->next = device_head;
        device_head = gdevice_interface;
    }
    return device_head;
}
#endif
struct gdevice *find_DeviceByKey(struct gdevice *device_head, int key)
{
    struct gdevice *pointer = NULL;

    if(device_head == NULL)
    {
        return NULL;
    }
    else
    {
        pointer = device_head;
        while(pointer != NULL)
        {
            if(pointer->key == key)
            {
                return pointer;
            }
            pointer = pointer->next;
        }
    }
    return NULL;
}

int set_gpio_gedvice_status(struct gdevice *device)
{
    if(device == NULL)
    {
        return -1;
    }
    
    if(device->gpio_pin != -1){
        if(device->gpio_mode != -1){
            pinMode(device->gpio_pin, device->gpio_mode);   //配置引脚的输入输出模式
        }

        if(device->gpio_status != -1){
            digitalWrite(device->gpio_pin, device->gpio_status);    //配置引脚的高低电平状态
        }
    }
}