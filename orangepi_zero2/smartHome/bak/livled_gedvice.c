#include "livled_gdevice.h"

struct gdevice livled_gdev = {
    .dev_name = "livled",
    .key = 0x41,
    .gpio_pin = 17,
    .gpio_mode = OUTPUT,
    .gpio_status = HIGH,
    .check_face_status = 0,
    .check_voice_status = 0
};

struct gdevice *add_livled_gdevInLink(struct gdevice *device_head)
{
    return add_DeviceInLink(device_head, &livled_gdev);
}