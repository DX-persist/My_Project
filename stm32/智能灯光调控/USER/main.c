#include "sys.h"                //头文件
#include "delay.h"
#include "led.h"
#include "Key.h"
#include "Buzzer.h"
#include "OLED.h"
#include "AD.h"
#include "MyRTC.h"
#include "dht11.h"
#include "Serial.h"
#include "PWM.h"
#include "Serial2.h"
#include <stdio.h>
#include <string.h>
#include "BODY_HW.h"
uint32_t bufe[5];           //存储传感器采集的数据
uint16_t AD4;    //存储4路ADC值
uint8_t RxData;      //蓝牙接收到的数据

uint32_t GuangYu = 40;     //光照强度阈值下限
uint32_t GuangYu1 = 20, GuangYu2 = 40, GuangYu3 = 60, GuangYu4 = 80;               //光照强度阈值上限
u8 temp, humi;                //存放温湿度
u8 state, state2, state2_1, state2_2, state3, state4, state5, state6, state7;      //按键状态标志
u8 t = 0;                       //传感器读取时间间隔
uint8_t KeyNum;                      //存储按键值
u8 k1, k2, k3;
u16 value = 0;
u16 pre_value = 0;
void shoudong()
{

    if (Serial_GetRxFlag() == 1)  //蓝牙接收部分
    {
        RxData = Serial_GetRxData();     //蓝牙接收
        switch (RxData)   //控制阈值
        {
        case 1:
            state2++;
            if (state2 > 3)
            {
                state2 = 0;
            }
            break;
        case 2:
            state2_1++;
            if (state2_1 > 3)
            {
                state2_1 = 0;
            }
            break;
        case 3:
            state2_2++;
            if (state2_2 > 3)
            {
                state2_2 = 0;
            }
            break;
        }
    }

    if (KeyNum == 2)        //按键PB0控制窗户开关
    {
        delay_ms(20);
        if (KeyNum == 2)
        {
            state2++;
            if (state2 > 3)
            {
                state2 = 0;
            }
        }
    }

    switch (state2)
    {
    case 0:
        PWM_SetCompare1(100);    //全灭
        PWM_SetCompare2(100);

        break;
    case 1:
        PWM_SetCompare1(0);    //暖光
        PWM_SetCompare2(100);
        break;
    case 2:
        PWM_SetCompare1(100);    //白光
        PWM_SetCompare2(0);
        break;
    case 3:
        PWM_SetCompare1(0);    //全亮
        PWM_SetCompare2(0);
        break;
    default:
        break;
    }
    if (KeyNum == 3)        //按键PB0控制窗户开关
    {
        delay_ms(20);
        if (KeyNum == 3)
        {
            state2_1++;
            if (state2_1 > 3)
            {
                state2_1 = 0;
            }
        }
    }

    switch (state2_1)
    {
    case 0:
        GPIO_SetBits(GPIOA, GPIO_Pin_6);                        //PB.5 输出高
        GPIO_SetBits(GPIOA, GPIO_Pin_7);

        break;
    case 1:
        GPIO_ResetBits(GPIOA, GPIO_Pin_6);                      //PB.5 输出高
        GPIO_SetBits(GPIOA, GPIO_Pin_7);
        break;
    case 2:
        GPIO_SetBits(GPIOA, GPIO_Pin_6);                        //PB.5 输出高
        GPIO_ResetBits(GPIOA, GPIO_Pin_7);
        break;
    case 3:
        GPIO_ResetBits(GPIOA, GPIO_Pin_6);                      //PB.5 输出高
        GPIO_ResetBits(GPIOA, GPIO_Pin_7);
        break;
    default:
        break;
    }
    if (KeyNum == 4)        //按键PB0控制窗户开关
    {
        delay_ms(20);
        if (KeyNum == 4)
        {
            state2_2++;
            if (state2_2 > 3)
            {
                state2_2 = 0;
            }
        }
    }

    switch (state2_2)
    {
    case 0:
        GPIO_SetBits(GPIOB, GPIO_Pin_0);                        //PB.5 输出高
        GPIO_SetBits(GPIOB, GPIO_Pin_1);

        break;
    case 1:
        GPIO_ResetBits(GPIOB, GPIO_Pin_0);                      //PB.5 输出高
        GPIO_SetBits(GPIOB, GPIO_Pin_1);
        break;
    case 2:
        GPIO_SetBits(GPIOB, GPIO_Pin_0);                        //PB.5 输出高
        GPIO_ResetBits(GPIOB, GPIO_Pin_1);
        break;
    case 3:
        GPIO_ResetBits(GPIOB, GPIO_Pin_0);                      //PB.5 输出高
        GPIO_ResetBits(GPIOB, GPIO_Pin_1);
        break;
    default:
        break;
    }

}
void zhidong()
{
    if (value == 1)
    {
        if (KeyNum == 2)        //按键PB0控制窗户开关
        {
            delay_ms(20);
            if (KeyNum == 2)
            {
                state5++;
                if (state5 > 1)
                {
                    state5 = 0;
                }
            }
        }

        if (state5 == 0)
        {
            k1 = 0;

        }
        if (state5 == 1)
        {
            k1 = 1;
        }
        if (KeyNum == 3)        //按键PB0控制窗户开关
        {
            delay_ms(20);
            if (KeyNum == 3)
            {
                state6++;
                if (state6 > 1)
                {
                    state6 = 0;
                }
            }
        }

        if (state6 == 0)
        {
            k2 = 0;

        }
        if (state6 == 1)
        {
            k2 = 1;
        }

        if (KeyNum == 4)        //按键PB0控制窗户开关
        {
            delay_ms(20);
            if (KeyNum == 4)
            {
                state7++;
                if (state7 > 1)
                {
                    state7 = 0;
                }
            }
        }

        if (state7 == 0)
        {
            k3 = 0;

        }
        if (state7 == 1)
        {
            k3 = 1;
        }




        if (k1 == 1)
        {
            switch (state4)
            {
            case 0:
                PWM_SetCompare1(100);    //全灭
                PWM_SetCompare2(100);

                break;
            case 1:
                PWM_SetCompare1(0);    //暖光
                PWM_SetCompare2(100);
                break;
            case 2:
                PWM_SetCompare1(100);    //白光
                PWM_SetCompare2(0);
                break;
            case 3:
                PWM_SetCompare1(0);    //全亮
                PWM_SetCompare2(0);
                break;
            default:
                break;
            }
        }
        else
        {
            PWM_SetCompare1(100);    //全灭
            PWM_SetCompare2(100);
        }

        if (k2 == 1)
        {
            switch (state4)
            {
            case 0:
                GPIO_SetBits(GPIOA, GPIO_Pin_6);                        //PB.5 输出高
                GPIO_SetBits(GPIOA, GPIO_Pin_7);

                break;
            case 1:
                GPIO_ResetBits(GPIOA, GPIO_Pin_6);                      //PB.5 输出高
                GPIO_SetBits(GPIOA, GPIO_Pin_7);
                break;
            case 2:
                GPIO_SetBits(GPIOA, GPIO_Pin_6);                        //PB.5 输出高
                GPIO_ResetBits(GPIOA, GPIO_Pin_7);
                break;
            case 3:
                GPIO_ResetBits(GPIOA, GPIO_Pin_6);                      //PB.5 输出高
                GPIO_ResetBits(GPIOA, GPIO_Pin_7);
                break;
            default:
                break;
            }
        }
        else
        {
            GPIO_SetBits(GPIOA, GPIO_Pin_6);                        //PB.5 输出高
            GPIO_SetBits(GPIOA, GPIO_Pin_7);
        }
        if (k3 == 1)
        {
            switch (state4)
            {
            case 0:
                GPIO_SetBits(GPIOB, GPIO_Pin_0);                        //PB.5 输出高
                GPIO_SetBits(GPIOB, GPIO_Pin_1);

                break;
            case 1:
                GPIO_ResetBits(GPIOB, GPIO_Pin_0);                      //PB.5 输出高
                GPIO_SetBits(GPIOB, GPIO_Pin_1);
                break;
            case 2:
                GPIO_SetBits(GPIOB, GPIO_Pin_0);                        //PB.5 输出高
                GPIO_ResetBits(GPIOB, GPIO_Pin_1);
                break;
            case 3:
                GPIO_ResetBits(GPIOB, GPIO_Pin_0);                      //PB.5 输出高
                GPIO_ResetBits(GPIOB, GPIO_Pin_1);
                break;
            default:
                break;
            }
        }
        else
        {
            GPIO_SetBits(GPIOB, GPIO_Pin_0);                        //PB.5 输出高
            GPIO_SetBits(GPIOB, GPIO_Pin_1);
        }
    }
    else
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_0);                        //PB.5 输出高
        GPIO_SetBits(GPIOB, GPIO_Pin_1);
        GPIO_SetBits(GPIOA, GPIO_Pin_6);                        //PB.5 输出高
        GPIO_SetBits(GPIOA, GPIO_Pin_7);
        PWM_SetCompare1(100);    //全灭
        PWM_SetCompare2(100);
    }

}




void YuYingMode()   //先说小杰唤醒，然后说打开窗户和关闭窗户
{
    if (Serial2_RxFlag == 1)        //串口接收到数据包的标志位，若是收到数据包，会置1
    {
        if (strcmp(Serial2_RxPacket, "WD_OFF") == 0)
        {
            GPIO_SetBits(GPIOB, GPIO_Pin_0);                        //PB.5 输出高
            GPIO_SetBits(GPIOB, GPIO_Pin_1);
        }
        else if (strcmp(Serial2_RxPacket, "WD_ON") == 0)
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_0);                      //PB.5 输出高
            GPIO_ResetBits(GPIOB, GPIO_Pin_1);
        }
        else if (strcmp(Serial2_RxPacket, "WDN_ON") == 0)
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_0);                      //PB.5 输出高
            GPIO_SetBits(GPIOB, GPIO_Pin_1);
        }
        else if (strcmp(Serial2_RxPacket, "WDB_ON") == 0)
        {
            GPIO_SetBits(GPIOB, GPIO_Pin_0);                        //PB.5 输出高
            GPIO_ResetBits(GPIOB, GPIO_Pin_1);
        }

        if (strcmp(Serial2_RxPacket, "KD_OFF") == 0)
        {
            PWM_SetCompare1(100);    //全灭
            PWM_SetCompare2(100);
        }
        else if (strcmp(Serial2_RxPacket, "KD_ON") == 0)
        {
            PWM_SetCompare1(0);    //全亮
            PWM_SetCompare2(0);
        }
        else if (strcmp(Serial2_RxPacket, "KDN_ON") == 0)
        {
            PWM_SetCompare1(100);    //白光
            PWM_SetCompare2(0);
        }
        else if (strcmp(Serial2_RxPacket, "KDB_ON") == 0)
        {
            PWM_SetCompare1(0);    //全亮
            PWM_SetCompare2(0);
        }

        if (strcmp(Serial2_RxPacket, "CD_OFF") == 0)
        {
            GPIO_SetBits(GPIOA, GPIO_Pin_6);                        //PB.5 输出高
            GPIO_SetBits(GPIOA, GPIO_Pin_7);
        }
        else if (strcmp(Serial2_RxPacket, "CD_ON") == 0)
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_6);                        //PB.5 输出高
            GPIO_ResetBits(GPIOA, GPIO_Pin_7);
        }
        else if (strcmp(Serial2_RxPacket, "CDN_ON") == 0)
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_6);                      //PB.5 输出高
            GPIO_SetBits(GPIOA, GPIO_Pin_7);
        }
        else if (strcmp(Serial2_RxPacket, "CDB_ON") == 0)
        {
            GPIO_SetBits(GPIOA, GPIO_Pin_6);                        //PB.5 输出高
            GPIO_ResetBits(GPIOA, GPIO_Pin_7);
        }
        Serial2_RxFlag = 0; //将标志位清零，不清零就接收不到下一个数据包了
    }
}
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    delay_init();      //延时函数初始化

    LED_Init();            //初始化与LED连接的硬件接口
    Buzzer_Init();             //下面为外设初始化
    OLED_Init();
    Key_Init();
    AD_Init();
    Serial_Init();   //串口1初始化
    MyRTC_Init();
    PWM_Init();
    PWM1_Init();  //PWM初始化
    Serial2_Init();   //串口2初始化(语音识别模块)


    OLED_ShowChinese(2, 1, 53);
    OLED_ShowChinese(2, 2, 54);
    OLED_ShowString(2, 5, ":");

    OLED_ShowChinese(2, 6, 75);
    OLED_ShowChinese(2, 7, 76);
    OLED_ShowString(2, 15, ":");

    OLED_ShowChinese(3, 1, 69);
    OLED_ShowString(3, 3, ":");

    OLED_ShowChinese(3, 4, 71);
    OLED_ShowString(3, 9, ":");

    OLED_ShowChinese(3, 7, 73);
    OLED_ShowString(3, 15, ":");


    OLED_ShowChinese(4, 1, 67);
    OLED_ShowChinese(4, 2, 68);
    OLED_ShowString(4, 5, ":");

    OLED_ShowString(1, 5, "XX:XX:XX");
    while (1)
    {
        MyRTC_ReadTime();    //读取时间（每一个页面都有时间显示）
        OLED_ShowNum(1, 5, MyRTC_Time[3], 2);    //时
        OLED_ShowNum(1, 8, MyRTC_Time[4], 2);    //分
        OLED_ShowNum(1, 11, MyRTC_Time[5], 2);   //秒
        OLED_ShowNum(4, 6, GuangYu1, 2);
        OLED_ShowNum(4, 9, GuangYu2, 2);
        OLED_ShowNum(4, 12, GuangYu3, 2);
        OLED_ShowNum(4, 15, GuangYu4, 2);
        OLED_ShowNum(3, 4, k1, 1);
        OLED_ShowNum(3, 10, k2, 1);
        OLED_ShowNum(3, 16, k3, 1);
        if (t % 10 == 0)
        {
            AD4 = AD_GetValue(ADC_Channel_5);    //光敏传感器     PA4
            if (AD4 > 4000)AD4 = 4000;
            bufe[2] = (u8)(100 - (AD4 / 40));
            OLED_ShowNum(2, 6, bufe[2], 2);
            OLED_ShowString(2, 8, "%");

            if (bufe[2] < GuangYu1)  state4 = 4;
            else if (bufe[2] < GuangYu2)  state4 = 3;
            else if (bufe[2] < GuangYu3)  state4 = 2;
            else if (bufe[2] < GuangYu4)  state4 = 1;
            else if (bufe[2] < 100)  state4 = 0;
        }
        t++;

        value = BODY_HW_GetData();

        if (value != pre_value)
        {
            OLED_ShowNum(2, 16, value, 1);
        }

        pre_value = value;

        KeyNum = Key_GetNum();

        if (KeyNum == 1)
        {
            delay_ms(20);
            if (KeyNum == 1)
            {
                state++;
                if (state > 2)
                {
                    state = 0;
                }
            }
        }
        if (state == 0)    //自动模式
        {
            OLED_ShowChinese(1, 7, 51);
            OLED_ShowChinese(1, 8, 52);
            zhidong();
        }
        if (state == 1)    //手动模式
        {
            OLED_ShowChinese(1, 7, 18);
            OLED_ShowChinese(1, 8, 52);
            shoudong();
        }
        if (state == 2)    //语音模式
        {
            OLED_ShowChinese(1, 7, 57);
            OLED_ShowChinese(1, 8, 58);
            YuYingMode();
        }
    }
}
