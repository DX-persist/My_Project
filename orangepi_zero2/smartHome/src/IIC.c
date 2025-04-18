#include "oled.h"

void IIC_START()
{
    OLED_SDA_HIGH();        //IIC的起始信号就是当SCL为高电平的时候SDA由高电平转为低电平
    OLED_SCL_HIGH();
    OLED_SDA_LOW();
    OLED_SCL_LOW();         //将SCL拉低方便下一次进行数据传输
}

void IIC_STOP()
{
    OLED_SDA_LOW();         //IIC的停止信号就是当SCL为高电平的时候SDA由低电平转换为高电平
    OLED_SCL_HIGH();
    OLED_SDA_HIGH();
}

char IIC_ACK()
{
    char flag;

    OLED_SDA_HIGH();        //在进行一个字节的传输后将SDA和SCL拉高释放IIC总线
    OLED_SCL_HIGH();
    flag = digitalRead(OLED_SDA_Pin);   //当从机向主机发送应答信号的时候会将数据线SDA拉低表示应答
    OLED_SCL_LOW();         //将SCL拉低表示第九个脉冲的结束
}

void IIC_SendByte(char dataSend)
{
    int i;

    for(i=0;i<8;i++)
    {
        OLED_SCL_LOW();     //只有SCL为低电平的时候，SDA数据线上的电平状态在允许改变
        if(dataSend & 0x80)     //每次将数据的最高位取出来放到SDA数据线上
        {
            OLED_SDA_HIGH();
        }
        else
        {
            OLED_SDA_LOW();
        }
        OLED_SCL_HIGH();    //SCL时钟线拉高开始向从机传送数据
        OLED_SCL_LOW();     //SCL时钟线拉低为了下一次数据的赋值做准备
        dataSend = dataSend << 1;   //向左移一位，将数据的次高位进行发送
    }
}