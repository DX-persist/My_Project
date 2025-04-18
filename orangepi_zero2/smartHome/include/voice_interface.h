#ifndef __VOICE_INTERFACE_H_
#define __VOICE_INTERFACE_H_

//system defined
#include <pthread.h>
#include <stdio.h>
#include <string.h>

//user defined
#include "msg_queue.h"
#include "control.h"
#include "uart.h"
#include "global.h"

#define CMD_BUFFER_SIZE 6
#define SERIAL_INIT_FAILURE -1

extern struct control *add_VoiceInLink(struct control *control_head);

#endif