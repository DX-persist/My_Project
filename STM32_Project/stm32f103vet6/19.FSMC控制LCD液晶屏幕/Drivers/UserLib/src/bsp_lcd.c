#include "bsp_lcd.h"

typedef struct{
	GPIO_TypeDef 	*gpio_port;
	uint16_t 		gpio_pin;
	uint32_t 		rcc_clk;
}bsp_lcd_gpio_t;

typedef struct{
	uint32_t bank;
    uint32_t memory_type;
    uint32_t data_width;
    uint32_t write_enable;
    uint32_t extended_mode;
}bsp_lcd_fsmc_config_t;

typedef struct{
    uint32_t addr_setup;
    uint32_t addr_hold;
    uint32_t data_setup;
    uint32_t bus_turnaround;
    uint32_t clk_div;
    uint32_t data_latency;
	uint32_t access_mode;
}bsp_lcd_fsmc_timing_t;

typedef struct{
	bsp_lcd_gpio_t lcd_reset;
	bsp_lcd_gpio_t lcd_bl;
	bsp_lcd_gpio_t lcd_cs;
	bsp_lcd_gpio_t lcd_dc;

	bsp_lcd_fsmc_config_t fsmc_config;
	bsp_lcd_fsmc_timing_t fsmc_read_timing;
	bsp_lcd_fsmc_timing_t fsmc_write_timing;

}bsp_lcd_hw_t;

static const bsp_lcd_hw_t bsp_lcd_hw = {

	.lcd_reset 	= {GPIOE, GPIO_Pin_1, RCC_APB2Periph_GPIOE},
	.lcd_bl    	= {GPIOD, GPIO_Pin_12, RCC_APB2Periph_GPIOD},	
	.lcd_cs		= {GPIOD, GPIO_Pin_7, RCC_APB2Periph_GPIOD},
	.lcd_dc		= {GPIOD, GPIO_Pin_11, RCC_APB2Periph_GPIOD},
	
	.fsmc_config = {
		.bank = FSMC_Bank1_NORSRAM1,
		.memory_type = FSMC_MemoryType_SRAM,
		.data_width = FSMC_MemoryDataWidth_16b,
		.write_enable = FSMC_WriteOperation_Enable,
		.extended_mode = FSMC_ExtendedMode_Disable,
	},

	.fsmc_read_timing = {
		.addr_setup = 0x01,
		.data_setup = 0x04,
		.access_mode = FSMC_AccessMode_B,
		.addr_hold = 0x00,
		.bus_turnaround = 0x00,
		.clk_div = 0x00,
		.data_latency = 0x00,
	},

	.fsmc_write_timing = {
		.addr_setup = 0x01,
		.addr_hold = 0x00,
		.data_setup = 0x02,
		.bus_turnaround = 0x00,
		.clk_div = 0x00,
		.access_mode = FSMC_AccessMode_B,
	},
};

static void BSP_LCD_BL_Enable(void)
{
	const bsp_lcd_hw_t *hw = &bsp_lcd_hw;

	GPIO_ResetBits(hw->lcd_bl.gpio_port, hw->lcd_bl.gpio_pin);
}

static void BSP_LCD_Reset(void)
{
	const bsp_lcd_hw_t *hw = &bsp_lcd_hw;

	GPIO_ResetBits(hw->lcd_reset.gpio_port, hw->lcd_reset.gpio_pin);
	BSP_Delay_ms(5);
	GPIO_SetBits(hw->lcd_reset.gpio_port, hw->lcd_reset.gpio_pin);
	BSP_Delay_ms(10);
}

static void BSP_LCD_Private_Pins_Init(void)
{
	const bsp_lcd_hw_t *hw = &bsp_lcd_hw;
	/* 开启 GPIO 时钟 */
	RCC_APB2PeriphClockCmd(hw->lcd_reset.rcc_clk | 
						   hw->lcd_bl.rcc_clk | 
						   hw->lcd_cs.rcc_clk | 
						   hw->lcd_dc.rcc_clk, ENABLE);

	/* 初始化LCD*/
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = hw->lcd_reset.gpio_pin;
	GPIO_Init(hw->lcd_reset.gpio_port, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = hw->lcd_bl.gpio_pin;
	GPIO_Init(hw->lcd_bl.gpio_port, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = hw->lcd_cs.gpio_pin;
	GPIO_Init(hw->lcd_cs.gpio_port, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = hw->lcd_dc.gpio_pin;
	GPIO_Init(hw->lcd_dc.gpio_port, &GPIO_InitStruct);

	BSP_LCD_BL_Enable();
	BSP_LCD_Reset();
}


void BSP_LCD_Init(void)
{
	const bsp_lcd_hw_t *hw = &bsp_lcd_hw;
	FSMC_NORSRAMInitTypeDef FSMC_InitStruct;
	FSMC_NORSRAMTimingInitTypeDef readTiming;
	FSMC_NORSRAMTimingInitTypeDef writeTiming;

	BSP_LCD_Private_Pins_Init();
	
	readTiming.FSMC_AddressSetupTime = hw->fsmc_read_timing.addr_setup;
	readTiming.FSMC_AddressHoldTime = hw->fsmc_read_timing.addr_hold;
	readTiming.FSMC_DataSetupTime = hw->fsmc_read_timing.data_setup;
	readTiming.FSMC_BusTurnAroundDuration = hw->fsmc_read_timing.bus_turnaround;
	readTiming.FSMC_CLKDivision = hw->fsmc_read_timing.clk_div;
	readTiming.FSMC_DataLatency = hw->fsmc_read_timing.data_latency;
	readTiming.FSMC_AccessMode = hw->fsmc_read_timing.access_mode;

	writeTiming.FSMC_AddressSetupTime = hw->fsmc_write_timing.addr_setup;
	writeTiming.FSMC_AddressHoldTime = hw->fsmc_write_timing.addr_hold;
	writeTiming.FSMC_DataSetupTime = hw->fsmc_write_timing.data_setup;
	writeTiming.FSMC_BusTurnAroundDuration = hw->fsmc_write_timing.bus_turnaround;
	writeTiming.FSMC_CLKDivision = hw->fsmc_write_timing.clk_div;
	writeTiming.FSMC_DataLatency = hw->fsmc_write_timing.data_latency;
	writeTiming.FSMC_AccessMode = hw->fsmc_write_timing.access_mode;

	FSMC_InitStruct.FSMC_Bank = hw->fsmc_config.bank;
	FSMC_InitStruct.FSMC_MemoryType = hw->fsmc_config.memory_type;
	FSMC_InitStruct.FSMC_MemoryDataWidth = hw->fsmc_config.data_width;
	FSMC_InitStruct.FSMC_WriteOperation = hw->fsmc_config.write_enable;
	FSMC_InitStruct.FSMC_ExtendedMode = hw->fsmc_config.extended_mode;
	FSMC_InitStruct.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_InitStruct.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_InitStruct.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	FSMC_InitStruct.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_InitStruct.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_InitStruct.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_InitStruct.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_InitStruct.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_InitStruct.FSMC_ReadWriteTimingStruct = &readTiming;
	FSMC_InitStruct.FSMC_WriteTimingStruct = &writeTiming;

	BSP_FSMC_Core_Init(&FSMC_InitStruct);
}

static inline void BSP_LCD_WriteCmd(uint16_t cmd)
{
	*LCD_CMD_ADDR = cmd;
}

static inline void BSP_LCD_WriteData(uint16_t data)
{
	*LCD_DATA_ADDR = data;
}

static inline void BSP_LCD_ReadData(uint16_t *data)
{
	uint16_t temp = 0;
	
	temp = *LCD_DATA_ADDR;
	if(data != NULL){
		*data = temp;
	}
}

uint16_t BSP_LCD_Test(void)
{
	uint16_t data = 0;

	BSP_LCD_WriteCmd(0x0C);
	BSP_LCD_ReadData(NULL);
	BSP_LCD_ReadData(&data);

	return data;
}
