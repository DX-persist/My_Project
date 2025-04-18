#ifndef __SMOKE__INTERFACE__H
#define __SMOKE__INTERFACE__H

#include <wiringPi.h>
#include <pthread.h>
#include <unistd.h>

#include "control.h"
#include "global.h"
#include "msg_queue.h"

#define SMOKE_PIN 6
#define SMOKE_MODE INPUT

extern struct control *add_SmokeInLink(struct control *control_head);

#endif