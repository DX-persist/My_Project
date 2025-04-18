#ifndef __RECEIVE_INTERFACE__H
#define __RECEIVE_INTERFACE__H

#include <pthread.h>
#include <errno.h>

#include "control.h"
#include "socket.h"
#include "msg_queue.h"
#include "global.h"
#include "oled.h"
#include "face.h"
#include "livled_gdevice.h"
#include "bedled_gdevice.h"
#include "fan_gdevice.h"
#include "beep_gdevice.h"
#include "lock_gdevice.h"

extern struct control *add_ReceiveInLink(struct control *control_head);

#endif