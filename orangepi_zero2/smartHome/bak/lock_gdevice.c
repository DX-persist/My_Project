#include "lock_gdevice.h"

struct gdevice lock_gdev = {
    .dev_name = "lock",
    .key = 0x44,
    .gpio_pin = 22,
    .gpio_mode = OUTPUT,
    .gpio_status = HIGH,
    .check_face_status = 1,
    .check_voice_status = 1
};

struct gdevice *add_lock_gdevInLink(struct gdevice *device_head)
{
    return add_DeviceInLink(device_head, &lock_gdev);
}