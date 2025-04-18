#include "beep_gdevice.h"

struct gdevice beep_gdev = {
    .dev_name = "beep",
    .key = 0x45,
    .gpio_pin = 23,
    .gpio_mode = OUTPUT,
    .gpio_status = HIGH,
    .check_face_status = 0,
    .check_voice_status = 1
};

struct gdevice *add_beep_gdevInLink(struct gdevice *device_head)
{
    return add_DeviceInLink(device_head, &beep_gdev);
}