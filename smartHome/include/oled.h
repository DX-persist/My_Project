#ifndef __OLED_H
#define __OLED_H

#include <wiringPi.h>


#define OLED_SCL_Pin 7
#define OLED_SDA_Pin 8
#define OLED_RES_Pin 11

#define OLED_SDA_HIGH() digitalWrite(OLED_SDA_Pin,HIGH)      //配置SDA引脚输出高低电平状态的宏   
#define OLED_SDA_LOW() digitalWrite(OLED_SDA_Pin,LOW)

#define OLED_SCL_HIGH() digitalWrite(OLED_SCL_Pin,HIGH)
#define OLED_SCL_LOW() digitalWrite(OLED_SCL_Pin,LOW)

#define OLED_RES_HIGH() digitalWrite(OLED_RES_Pin,HIGH)
#define OLED_RES_LOW() digitalWrite(OLED_RES_Pin,LOW)

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

void IO_Init();
void OLED_WR_Byte(unsigned char dat,unsigned char mode);
void OLED_Clear(void) ;
void OLED_Set_Pos(unsigned char x, unsigned char y) ;
void OLED_ShowChar(unsigned char x,unsigned char y,unsigned char chr,unsigned char sizey);
unsigned long int oled_pow(unsigned char m,unsigned char n);
void OLED_ShowNum(unsigned char x,unsigned char y,unsigned long int num,unsigned char len,unsigned char sizey);
void OLED_ShowString(unsigned char x,unsigned char y,unsigned char *chr,unsigned char sizey);
void OLED_ShowChinese(unsigned char x,unsigned char y,unsigned char no,unsigned char sizey);
void OLED_DrawBMP(unsigned char x,unsigned char y,unsigned char sizex, unsigned char sizey,unsigned char BMP[]);
void OLED_Init(void);
void OLED_ColorTurn(unsigned char i);
void OLED_DisplayTurn(unsigned char i);
void OLED_Diaplay_Chinese();

#endif
