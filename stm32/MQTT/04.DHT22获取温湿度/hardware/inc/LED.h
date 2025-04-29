#ifndef _LED_H_
#define _LED_H_




typedef struct
{

	_Bool LED_Status;

} LED_INFO;

#define LED_ON		1
#define LED_OFF	0
#define LED_PORT GPIOB
#define LED_PIN GPIO_Pin_8

extern LED_INFO LED_info;


void LED_Init(void);

void LED_Set(_Bool status);


#endif
