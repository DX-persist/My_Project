#include "control.h"

struct control *add_InterfaceInLink(struct control *control_head, struct control *control_interface)
{
    if(control_head == NULL){
        control_head = control_interface;
    }
    else{
        control_interface->next = control_head;
        control_head = control_interface;
    }
}