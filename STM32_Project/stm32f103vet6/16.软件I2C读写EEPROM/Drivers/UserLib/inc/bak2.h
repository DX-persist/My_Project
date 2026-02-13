#ifndef BSP_EEPROM_H
#define BSP_EEPROM_H

#include "bsp_hi2c.h"
#include "bsp_usart.h"

#define BSP_EEPROM_I2C_ID			BSP_I2C1
#define BSP_EEPROM_WRITE_PAGE_MAX	8

#define BSP_EEPROM_WRITE_ADDR		0xA0
#define BSP_EEPROM_READ_ADDR		0xA1

extern void BSP_EEPROM_Init(void);
extern void BSP_EEPROM_WriteByte(uint8_t write_addr, uint8_t data, uint32_t timeout_ms);
extern void BSP_EEPROM_WritePage(uint8_t write_addr, uint8_t *buffer, uint8_t size, uint32_t timeout_ms);
extern void BSP_EEPROM_ReadRandom(uint8_t read_addr, uint8_t *data, uint32_t timeout_ms);
extern void BSP_EEPROM_ReadSequential(uint8_t read_addr, uint8_t *buffer, uint8_t size, uint32_t timeout_ms);
extern void BSP_EEPROM_WriteBuffer(uint16_t write_addr, uint8_t *buffer, uint16_t size, uint32_t timeout_ms);
extern void BSP_EEPROM_ReadBuffer(uint16_t read_addr, uint8_t *buffer, uint16_t size, uint32_t timeout_ms);

#endif
