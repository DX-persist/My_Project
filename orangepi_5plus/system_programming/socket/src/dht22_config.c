#include "dht22_config.h"
#include <unistd.h>

void delay_us_busy(unsigned int us)
{
	struct timespec start, now;

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);			//获取开始时间,CLOCK_MONOTONIC_RAW（不受网络时间协议调整影响的单调递增时钟）
	while(1){
		clock_gettime(CLOCK_MONOTONIC_RAW, &now);		//获取当前时间，若当前时间转换为微秒后大于等于用户所设定时间退出循环
		unsigned int elapsed = (now.tv_sec - start.tv_sec) * 1000000 + 
								(now.tv_nsec - start.tv_nsec) / 1000;
		if(elapsed >= us) break;
	}
}

static void DHT22_SetOutput(void)
{
	pinMode(DHT22_PIN, OUTPUT);
//	digitalWrite(DHT22_PIN, HIGH);
}

static void DHT22_SetInput(void)
{
	pinMode(DHT22_PIN, INPUT);
}

static void DHT22_Start(void)
{
	//将主机拉低至少800us表示将DHT22从休眠模式唤醒到高速模式
	DHT22_SetOutput();
	DHT22_SetPin(LOW);
	delay_us_busy(1000);
	//唤醒后释放总线，此时将该引脚配置为输入模式读取DHT22的应答信号
	DHT22_SetPin(HIGH);
	delay_us_busy(30);
	DHT22_SetInput();
}

static uint8_t DHT22_Wait_Response(void)
{
	uint8_t timeout = 0;

	//应答信号为80us的低电平后80us的高电平（最大持续时间不超过100us）

	//判断DHT22是否有下降沿产生表示从机给主机的应答信号
	while(DHT22_ReadPin() == HIGH && timeout <= 100){
		timeout++;
		delay_us_busy(1);
	}	
	if(timeout > 100)	return ACK_ERROR;
	
	//如果判断有下降沿产生则继续判断低电平持续时间
	timeout = 0;
	while(DHT22_ReadPin() == LOW && timeout <= 100){
		timeout++;
		delay_us_busy(1);
	}
	if(timeout > 100)	return ACK_TIMEOUT;

	//判断高电平的持续时间
	timeout = 0;
	while(DHT22_ReadPin() == HIGH && timeout <= 100){
		timeout++;
		delay_us_busy(1);
	}
	if(timeout > 100)	return ACK_TIMEOUT;

	return ACK_SUCCESS;
}

static uint8_t DHT22_ReadByte(void)
{
	uint8_t Byte = 0;
	//从机向主机返回的使50us低电平+26us/70us高电平,通过高电平的持续时间来判断数据位是0/1
	uint8_t timeout = 0;

	for(int i = 0; i < 8; i++){
		while(DHT22_ReadPin() == LOW && timeout <= 50){
			timeout++;
			delay_us_busy(1);
		}
		if(timeout > 50)	return 1;

		timeout = 0;
		Byte <<= 1;
		delay_us_busy(30);
		if(DHT22_ReadPin() == HIGH)
			Byte |= 1;
		while(DHT22_ReadPin() == HIGH);
	}
	return Byte;
}

uint8_t DHT22_ReadData(DHT22_Data_t *Data)
{
	DHT22_Start();

	if(DHT22_Wait_Response() != ACK_SUCCESS){
		fprintf(stderr, "No response from DHT22\n");
		return -1;
	}

	Data->humi_int = DHT22_ReadByte();
	Data->humi_dec = DHT22_ReadByte();
	Data->temp_int = DHT22_ReadByte();
	Data->temp_dec = DHT22_ReadByte();
	Data->checknum = DHT22_ReadByte();

	uint8_t sum = Data->humi_int + Data->humi_dec + Data->temp_int + Data->temp_dec;
	if(sum != Data->checknum){
		fprintf(stderr, "Checknum Error: %d != %d\n",sum, Data->checknum);
		return -2;
	}

	Data->humidity = (256 * Data->humi_int + Data->humi_dec) / 10.0f;
	Data->temperature = (256 * Data->temp_int + Data->temp_dec) / 10.0f;

	return 0;
}

void get_formatted_time(char *buffer, size_t len)
{
	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);
	strftime(buffer, len, "%Y年%m月%d日 %H:%M:%S",tm_info);
}