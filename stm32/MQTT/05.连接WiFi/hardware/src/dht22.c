#include "dht22.h"
#include "delay.h"

static void DHT22_SetOutput(void)
{
    RCC_APB2PeriphClockCmd(DHT22_RCC_CLOCK, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.GPIO_Pin = DHT22_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    
    GPIO_Init(DHT22_GPIO_PORT, &GPIO_InitStruct);
    DHT22_SetPin(1);
}

static void DHT22_SetInput(void)
{
    RCC_APB2PeriphClockCmd(DHT22_RCC_CLOCK, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.GPIO_Pin = DHT22_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    
    GPIO_Init(DHT22_GPIO_PORT, &GPIO_InitStruct);
}

static void DHT22_Start(void)
{
    DHT22_SetOutput();          //将DHT22引脚配置为输出模式并拉高
    DHT22_SetPin(0);            
    DelayMs(1);                //延时20ms，确保DHT22进入响应状态
    DHT22_SetPin(1);            
    DelayUs(30);                //延时30us，准备接收应答信号
    DHT22_SetInput();            //切换为输入模式等待响应
}

int8_t DHT22_Check_Response(void)
{
	uint8_t cnt = 0;
	//DHT22_SetInput();//设置IO为输入模式
	if (DHT_DQ_IN() == Bit_RESET) //检测是否有低电平
	{
		while((!DHT_DQ_IN()) && (cnt <= 85))//计算低电平持续时间，判断低电平是否应答超时，手册最大应答时间是85us
		{
			cnt ++; //每1us自加一次
			DelayUs(1);
		}
		if(cnt > 85) //大于85us则应答超时
		{
			return ACK_OVER_TIME;//响应超时
		}
		else //应答正常
		{
			cnt = 0;
		}
		/*低电平应答正常后到高电平                       */
		while(DHT_DQ_IN() && (cnt <= 85))//计算高电平持续时间，判断高电平是否应答超时，最大应答时间也是85us
		{
			cnt ++;
			DelayUs(1);
		}
		if(cnt > 85) //响应超时
		{
			return ACK_OVER_TIME;
		}
		else
		{
			cnt = 0;
			return ACK_SUCCESS;
		}
	}
	else
	{
		return ACK_ERROR;//应答错误，起始信号后没有应答信号返回
	}
}

uint8_t DHT22_Read_Byte(void)
{
	uint8_t i;
	uint8_t DHT22_Byte = 0;
	for(i = 0; i < 8; i++)
	{
		while(DHT_DQ_IN() == Bit_RESET);//等待低电平结束
		DelayUs(40); //等待40us，再判断IO口电平状态
		if(DHT_DQ_IN() == Bit_SET)// 40 us后仍为高电平则表示数据“1” 
		{
			/* 等待数据1的高电平结束 */
			while(DHT_DQ_IN() == Bit_SET);
 
			DHT22_Byte |= (uint8_t)(0x01 << (7-i));  //把第7-i位置1，MSB先行 
		}
		else	 // 40 us后为低电平则表示数据“0”
		{			   
			DHT22_Byte &= (uint8_t)~(0x01 << (7-i)); //把第7-i位置0，MSB先行
		}
	}
	
	return DHT22_Byte;//返回当前读取到的字节
}

uint8_t Read_DHT22_Data(DHT22_Data_TypeDef *Data)
{
	
	DHT22_Start();//发送起始信号
	if(DHT22_Check_Response() == ACK_SUCCESS) //接收应答成功
	{
		Data->humi_int = DHT22_Read_Byte();
        Data->humi_dec = DHT22_Read_Byte();
        Data->temp_int = DHT22_Read_Byte();
        Data->temp_dec = DHT22_Read_Byte();
        Data->check_num = DHT22_Read_Byte();
	}
	
	DHT22_SetOutput();//设置IO为输出模式
	GPIO_SetBits(GPIOA, GPIO_Pin_0);//释放总线
	
 
	if(Data->check_num == Data->humi_int + Data->humi_dec + 
                            Data->temp_int + Data->temp_dec)//校验正确
	{
        Data->humidity = (256 * Data->humi_int + Data->humi_dec) / 10.0f;
        Data->temperature = (256 * Data->temp_int + Data->temp_dec) / 10.0f;		
		return SUCCESS; //校验通过
	}
	else
		return ERROR;//校验不过
}
