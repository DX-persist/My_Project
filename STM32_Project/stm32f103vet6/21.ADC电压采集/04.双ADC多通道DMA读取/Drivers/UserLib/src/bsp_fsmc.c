#include "bsp_fsmc.h"

typedef struct{
	GPIO_TypeDef 	*gpio_port;
	uint16_t 		gpio_pin;
	uint32_t 		rcc_clk;
}bsp_fsmc_gpio_t;

typedef struct{
	bsp_fsmc_gpio_t fsmc_data_pins[16];
	bsp_fsmc_gpio_t fsmc_ctrl_pins[2];
	uint32_t fsmc_clk;
}bsp_fsmc_hw_t;

static const bsp_fsmc_hw_t bsp_fsmc_hw = {
	
	.fsmc_data_pins = {
		[0] = {GPIOD, GPIO_Pin_14, RCC_APB2Periph_GPIOD},
		[1] = {GPIOD, GPIO_Pin_15, RCC_APB2Periph_GPIOD},	
		[2] = {GPIOD, GPIO_Pin_0, RCC_APB2Periph_GPIOD},	
		[3] = {GPIOD, GPIO_Pin_1, RCC_APB2Periph_GPIOD},	
		[4] = {GPIOE, GPIO_Pin_7, RCC_APB2Periph_GPIOE},
		[5] = {GPIOE, GPIO_Pin_8, RCC_APB2Periph_GPIOE},
		[6] = {GPIOE, GPIO_Pin_9, RCC_APB2Periph_GPIOE},
		[7] = {GPIOE, GPIO_Pin_10, RCC_APB2Periph_GPIOE},
		[8] = {GPIOE, GPIO_Pin_11, RCC_APB2Periph_GPIOE},
		[9] = {GPIOE, GPIO_Pin_12, RCC_APB2Periph_GPIOE},
		[10] = {GPIOE, GPIO_Pin_13, RCC_APB2Periph_GPIOE},
		[11] = {GPIOE, GPIO_Pin_14, RCC_APB2Periph_GPIOE},
		[12] = {GPIOE, GPIO_Pin_15, RCC_APB2Periph_GPIOE},
		[13] = {GPIOD, GPIO_Pin_8, RCC_APB2Periph_GPIOD},
		[14] = {GPIOD, GPIO_Pin_9, RCC_APB2Periph_GPIOD},
		[15] = {GPIOD, GPIO_Pin_10, RCC_APB2Periph_GPIOD},
	},

	.fsmc_ctrl_pins = {
		[0] = {GPIOD, GPIO_Pin_4, RCC_APB2Periph_GPIOD},			/**< FSMC 的读使能,低电平有效 */	
		[1] = {GPIOD, GPIO_Pin_5, RCC_APB2Periph_GPIOD}				/**< FSMC 的写使能,低电平有效 */
	},

	.fsmc_clk = RCC_AHBPeriph_FSMC
};

static void BSP_FSMC_Enable_Clk(const bsp_fsmc_gpio_t *pins, uint8_t size)
{
	uint32_t enable_clk = 0;
	for(int i = 0; i < size; i++){
		enable_clk |= pins[i].rcc_clk;
	}
	RCC_APB2PeriphClockCmd(enable_clk, ENABLE);
}

static void BSP_FSMC_Pins_Config(const bsp_fsmc_gpio_t *pins, uint8_t size)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;	
	for(int i = 0; i < size; i++){
		GPIO_InitStruct.GPIO_Pin = pins[i].gpio_pin;
		GPIO_Init(pins[i].gpio_port, &GPIO_InitStruct);
	}
}

static void BSP_FSMC_SharePins_Init(void)
{
	uint8_t fsmc_data_pins_size = sizeof(bsp_fsmc_hw.fsmc_data_pins) / sizeof(bsp_fsmc_hw.fsmc_data_pins[0]);
	uint8_t fsmc_ctrl_pins_size = sizeof(bsp_fsmc_hw.fsmc_ctrl_pins) / sizeof(bsp_fsmc_hw.fsmc_ctrl_pins[0]);
	/* 初始化FSMC 的引脚所在端口的时钟 */
	BSP_FSMC_Enable_Clk(bsp_fsmc_hw.fsmc_data_pins, fsmc_data_pins_size);
	BSP_FSMC_Enable_Clk(bsp_fsmc_hw.fsmc_ctrl_pins, fsmc_ctrl_pins_size);
	/* 开启 FSMC 时钟 */
	RCC_AHBPeriphClockCmd(bsp_fsmc_hw.fsmc_clk, ENABLE);

	/* 初始化FSMC 的引脚模式为复用推挽输出 */
	BSP_FSMC_Pins_Config(bsp_fsmc_hw.fsmc_data_pins, fsmc_data_pins_size);
	BSP_FSMC_Pins_Config(bsp_fsmc_hw.fsmc_ctrl_pins, fsmc_ctrl_pins_size);
}

void BSP_FSMC_Core_Init(FSMC_NORSRAMInitTypeDef *fsmc_init)
{
	BSP_FSMC_SharePins_Init();

	/* 初始化FSMC_NORSRAMInitTypeDef 结构体 */
	FSMC_NORSRAMInit(fsmc_init);
	/* 开启FSMC 外设 */
	FSMC_NORSRAMCmd(fsmc_init->FSMC_Bank, ENABLE);
}
