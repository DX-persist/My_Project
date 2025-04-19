#include "bedled_gdevice.h"

struct gdevice bedled_gdev = {
    .dev_name = "bedled",
    .key = 0x42,
    .gpio_pin = 19,
    .gpio_mode = OUTPUT,
    .gpio_status = HIGH,
    .check_face_status = 0,
    .check_voice_status = 0
};

struct gdevice *add_bedled_gdevInLink(struct gdevice *device_head)
{
    return add_DeviceInLink(device_head, &bedled_gdev);
}