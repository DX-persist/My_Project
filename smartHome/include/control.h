#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <stdio.h>

struct control
{
    char control_name[128];
    int (*init)(void);
    void (*final)(void);
    void *(*get)(void *arg);
    void *(*set)(void *arg);
    
    struct control *next;
};

extern struct control *add_InterfaceInLink(struct control *control_head, struct control *control_interface);


#endif