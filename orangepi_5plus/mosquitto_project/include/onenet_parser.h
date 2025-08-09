#ifndef _ONENET_PARSER_H
#define _ONENET_PARSER_H

#include "onenet_config.h"

typedef struct{
	char id[32];
	char led_cmd[32];
	bool led_status;
}device_command_t;

extern void handle_control_message(const char *payload, onenet_client_t *client);

#endif
