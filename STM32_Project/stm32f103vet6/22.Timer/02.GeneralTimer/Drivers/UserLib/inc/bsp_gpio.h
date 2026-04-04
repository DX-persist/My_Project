#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "stm32f10x.h"
#include <stdint.h>

/**
 * @file bsp_gpio.h
 * @brief GPIO BSP abstraction layer for STM32F10x.
 *
 * This file provides a lightweight GPIO abstraction structure and
 * basic inline functions for GPIO read and write operations.
 */

/**
 * @brief GPIO device description structure
 *
 * This structure describes a GPIO pin on STM32, including
 * the port base address, pin mask, and corresponding RCC clock.
 */
typedef struct{
	GPIO_TypeDef *gpio_port; /**< GPIO port base address (e.g., GPIOA, GPIOB) */
	uint16_t gpio_pin;       /**< GPIO pin mask (e.g., GPIO_Pin_0) */
	uint32_t rcc_clk;        /**< RCC clock for the GPIO port (e.g., RCC_APB2Periph_GPIOA) */
} bsp_gpio_t;

/**
 * @brief Write a logic level to a GPIO pin
 *
 * This function sets or resets a GPIO pin using the BSRR register,
 * which guarantees atomic operation.
 *
 * @param gpio  Pointer to the GPIO configuration structure
 * @param level Output level
 *              - 1 : Set pin high
 *              - 0 : Set pin low
 */
static inline void BSP_GPIO_WritePin(const bsp_gpio_t *gpio, uint8_t level)
{
	if(level){
		gpio->gpio_port->BSRR = ((uint32_t)gpio->gpio_pin);
	}else{
		gpio->gpio_port->BSRR = (((uint32_t)gpio->gpio_pin) << 16);
	}
}

/**
 * @brief Read the logic level of a GPIO pin
 *
 * This function reads the input data register (IDR) of the GPIO port
 * and returns the state of the specified pin.
 *
 * @param gpio Pointer to the GPIO configuration structure
 *
 * @return uint16_t
 * @retval Non-zero  The pin is high
 * @retval 0         The pin is low
 */
static inline uint16_t BSP_GPIO_ReadPin(const bsp_gpio_t *gpio)
{
	return (gpio->gpio_port->IDR & gpio->gpio_pin);
}

#endif