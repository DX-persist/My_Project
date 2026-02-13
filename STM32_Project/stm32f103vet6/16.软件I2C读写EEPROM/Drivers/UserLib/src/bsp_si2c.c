#include "bsp_si2c.h"

typedef struct{
	GPIO_TypeDef scl_port;
	uint16_t scl_pin;
	uint32_t scl_clk;

	GPIO_TypeDef sda_port;
	uint16_t sda_pin;
	uint32_t sda_clk
}bsp_i2c_sw_t;

static const bsp_i2c_sw_t bsp_i2c_sw[BSP_SI2C_MAX] = {
	[BSP_SI2C1] = {
		.scl_port = GPIOB,
		.scl_pin = GPIO_Pin_6,
		.scl_clk = RCC_APB2Periph_GPIOB,

		.sda_port = GPIOB,
		.sda_pin = GPIO_Pin_7,
		.sda_clk = RCC_APB2Periph_GPIOB
	},

	[BSP_SI2C2] = {
		.scl_port = GPIOC,
		.scl_pin = GPIO_Pin_6,
		.scl_clk = RCC_APB2Periph_GPIOC,

		.sda_port = GPIOC,
		.sda_pin = GPIO_Pin_7,
		.sda_clk = RCC_APB2Periph_GPIOC
		
	}
};
