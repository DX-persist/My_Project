#include "fan_gdevice.h"

struct gdevice fan_gdev = {
    .dev_name = "fan",
    .key = 0x43,
    .gpio_pin = 20,
    .gpio_mode = OUTPUT,
    .gpio_status = HIGH,
    .check_face_status = 0,
    .check_voice_status = 0
};

struct gdevice *add_fan_gdevInLink(struct gdevice *device_head)
{
    return add_DeviceInLink(device_head, &fan_gdev);
}