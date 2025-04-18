#ifndef __SOCKET_INTERFACE__H
#define __SOCKET_INTERFACE__H

#include <pthread.h>

#include "control.h"
#include "socket.h"
#include "msg_queue.h"
#include "global.h"

extern struct control *add_SocketInLink(struct control *control_head);

#endif