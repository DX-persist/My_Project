#ifndef __DHT22_CONFIG_H
#define __DHT22_CONFIG_H

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <wiringPi.h>

#define DHT22_PIN 3
#define DHT22_SetPin(x) digitalWrite(DHT22_PIN, (int)x)
#define DHT22_ReadPin() digitalRead(DHT22_PIN)
#define ACK_ERROR   -1
#define ACK_SUCCESS  0
#define ACK_TIMEOUT  1

typedef struct{
    uint8_t humi_int;
    uint8_t humi_dec;
    uint8_t temp_int;
    uint8_t temp_dec;
    uint8_t checknum;
    uint8_t sum;
    float humidity;
    float temperature;
}DHT22_Data_t;

extern void delay_us_busy(unsigned int us);
extern void get_formatted_time(char *buffer, size_t len);
extern uint8_t DHT22_ReadData(DHT22_Data_t *Data);

#endif