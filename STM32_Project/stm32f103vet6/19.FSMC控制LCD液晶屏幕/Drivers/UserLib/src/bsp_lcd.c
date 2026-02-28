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

static inline void BSP_LCD_WriteCmd(uint16_t cmd)
{
	*LCD_CMD_ADDR = cmd;
}

static inline void BSP_LCD_WriteData(uint16_t data)
{
	*LCD_DATA_ADDR = data;
}

static inline uint16_t BSP_LCD_ReadData(void)
{
	return *LCD_DATA_ADDR;
}

/**
 * @brief  开辟 LCD 的显示窗口
 */
static void BSP_LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    BSP_LCD_WriteCmd(LCD_SET_COL_CMD); 		/* 列地址设置 0x2A */
    BSP_LCD_WriteData(x1 >> 8);				/**< 设置起始 x 的高位地址 */
    BSP_LCD_WriteData(x1 & 0xFF);			/**< 设置起始 x 的低位地址 */
    BSP_LCD_WriteData(x2 >> 8);				/**< 设置结束 x 的高位地址 */
    BSP_LCD_WriteData(x2 & 0xFF);			/**< 设置结束 x 的低位地址 */
    BSP_LCD_WriteCmd(LCD_SET_PAGE_CMD); 	/* 页地址设置 0x2B */
    BSP_LCD_WriteData(y1 >> 8);				/**< 设置起始 y 的高位地址 */
    BSP_LCD_WriteData(y1 & 0xFF);			/**< 设置起始 y 的低位地址 */
    BSP_LCD_WriteData(y2 >> 8);				/**< 设置结束 y 的高位地址 */
    BSP_LCD_WriteData(y2 & 0xFF);			/**< 设置结束 y 的低位地址 */
}
/**
 * @brief  设置操作光标位置
 */
static void BSP_LCD_SetCursor(uint16_t x, uint16_t y)
{
    BSP_LCD_SetWindow(x, y, x, y);
}
/* ------------------- 对外暴露的绘图接口 ------------------- */
/**
 * @brief  在指定像素点画一个带有颜色的点
 */
void BSP_LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if(x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    
    BSP_LCD_SetCursor(x, y);
    BSP_LCD_WriteCmd(LCD_SET_MEM_CMD);      /* 发送写显存命令 */
    BSP_LCD_WriteData(color);    /* 写入颜色数据 */
}
/**
 * @brief  以特定的颜色清空整个屏幕
 */
void BSP_LCD_Clear(uint16_t color)
{
    uint32_t i = 0;
    uint32_t total_points = LCD_WIDTH * LCD_HEIGHT;
    /* 开辟全屏窗口 */
    BSP_LCD_SetWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    
    BSP_LCD_WriteCmd(LCD_SET_MEM_CMD); /* 准备连续写入显存 */
    for (i = 0; i < total_points; i++) {
        BSP_LCD_WriteData(color);
    }
}
/**
 * @brief  画线函数 (基于 Bresenham 算法)
 */
void BSP_LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1; 
    delta_y = y2 - y1; 
    uRow = x1; 
    uCol = y1; 
    if (delta_x > 0) {
        incx = 1; /* 向右 */
    } else if (delta_x == 0) {
        incx = 0; /* 垂直线 */
    } else {
        incx = -1; /* 向左 */
        delta_x = -delta_x;
    }
    if (delta_y > 0) {
        incy = 1; /* 向下 */
    } else if (delta_y == 0) {
        incy = 0; /* 水平线 */
    } else {
        incy = -1; /* 向上 */
        delta_y = -delta_y;
    }
    if (delta_x > delta_y) distance = delta_x; 
    else distance = delta_y; 
    for (int i = 0; i <= distance + 1; i++) {
        BSP_LCD_DrawPixel(uRow, uCol, color);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}
/**
 * @brief  画矩形框
 */
void BSP_LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    BSP_LCD_DrawLine(x, y, x + width - 1, y, color);                           /* 上边 */
    BSP_LCD_DrawLine(x, y + height - 1, x + width - 1, y + height - 1, color); /* 下边 */
    BSP_LCD_DrawLine(x, y, x, y + height - 1, color);                          /* 左边 */
    BSP_LCD_DrawLine(x + width - 1, y, x + width - 1, y + height - 1, color);  /* 右边 */
}
/**
 * @brief  画圆函数 (基于 Bresenham 算法)
 */
void BSP_LCD_DrawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int a = 0, b = r;
    int di = 3 - (r << 1); /* 判断下个点位置的标志 */
    
    while (a <= b) {
        BSP_LCD_DrawPixel(x0 + a, y0 - b, color); /* 5 区 */
        BSP_LCD_DrawPixel(x0 + b, y0 - a, color); /* 0 区 */
        BSP_LCD_DrawPixel(x0 + b, y0 + a, color); /* 7 区 */
        BSP_LCD_DrawPixel(x0 + a, y0 + b, color); /* 6 区 */
        BSP_LCD_DrawPixel(x0 - a, y0 + b, color); /* 1 区 */
        BSP_LCD_DrawPixel(x0 - b, y0 + a, color); /* 2 区 */
        BSP_LCD_DrawPixel(x0 - a, y0 - b, color); /* 4 区 */
        BSP_LCD_DrawPixel(x0 - b, y0 - a, color); /* 3 区 */
        
        a++;
        if (di < 0) {
            di += 4 * a + 6;
        } else {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

static void BSP_LCD_BL_Config(FunctionalState NewState)
{
	const bsp_lcd_hw_t *hw = &bsp_lcd_hw;

	/* 直接使用寄存器操作，防止函数调用产生不必要的开销 */
	if(NewState == ENABLE){
		hw->lcd_bl.gpio_port->BSRR = (((uint32_t)hw->lcd_bl.gpio_pin) << 16);
	}else{ 
		hw->lcd_bl.gpio_port->BSRR = ((uint32_t)hw->lcd_bl.gpio_pin);	
	}
}

static void BSP_LCD_ResetConfig(void)
{
	const bsp_lcd_hw_t *hw = &bsp_lcd_hw;

	hw->lcd_reset.gpio_port->BSRR = (((uint32_t)hw->lcd_reset.gpio_pin) << 16);
	BSP_Delay_ms(5);
	hw->lcd_reset.gpio_port->BSRR = ((uint32_t)hw->lcd_reset.gpio_pin);
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

	BSP_LCD_BL_Config(ENABLE);
	BSP_LCD_ResetConfig();
}

static uint32_t BSP_LCD_ReadID(void)
{
    uint32_t LCD_ID = 0;

    BSP_LCD_WriteCmd(0xD3);    /**< 发送读取 ID 指令: 0xD3 */

    /* 第一个读取周期是 Dummy (假读)，芯片还不给有用的数据，直接丢弃 */
    BSP_LCD_ReadData();        

    /* 第二个周期的读出值通常是 0x00 (参数1) */
    LCD_ID = BSP_LCD_ReadData() & 0x00FF; 

    /* 第三个周期的读出值 (参数2) ，ILI9341 读出来的就是 0x93 */
    LCD_ID <<= 8;
    LCD_ID |= (BSP_LCD_ReadData() & 0x00FF);
    
    /* 第四个周期的读出值 (参数3) ，ILI9341 读出来的就是 0x41 */
    LCD_ID <<= 8;
    LCD_ID |= (BSP_LCD_ReadData() & 0x00FF);

    return LCD_ID;
}

static void BSP_LCD_REG_Config(void)
{ 
  if(BSP_LCD_ReadID() == ILI9341_LCD_ID){

	/* Power control B (CFh) */
	BSP_LCD_WriteCmd(0xCF);
	BSP_LCD_WriteData(0x00);
	BSP_LCD_WriteData(0x81);
	BSP_LCD_WriteData(0x30);

	/* Power on sequence control (EDh) */
	BSP_LCD_WriteCmd(0xED);
	BSP_LCD_WriteData(0x64);
	BSP_LCD_WriteData(0x03);
	BSP_LCD_WriteData(0x12);
	BSP_LCD_WriteData(0x81);

	/* Driver timing control A (E8h) */
	BSP_LCD_WriteCmd(0xE8);
	BSP_LCD_WriteData(0x85);
	BSP_LCD_WriteData(0x10);
	BSP_LCD_WriteData(0x78);

	/* Power control A (CBh) */
	BSP_LCD_WriteCmd(0xCB);
	BSP_LCD_WriteData(0x39);
	BSP_LCD_WriteData(0x2C);
	BSP_LCD_WriteData(0x00);
	BSP_LCD_WriteData(0x34);
	BSP_LCD_WriteData(0x06);/*原为0x02，改为0x06可避免白屏时出现条纹*/

	/* Pump ratio control (F7h) */
	BSP_LCD_WriteCmd(0xF7);
	BSP_LCD_WriteData(0x20);

	/* Driver timing control B (EAh) */
	BSP_LCD_WriteCmd(0xEA);
	BSP_LCD_WriteData(0x00);
	BSP_LCD_WriteData(0x00);

	/* Frame rate control (B1h) */
	BSP_LCD_WriteCmd(0xB1);
	BSP_LCD_WriteData(0x00);
	BSP_LCD_WriteData(0x1B);

	/* Display function control (B6h) */
	BSP_LCD_WriteCmd(0xB6);
	BSP_LCD_WriteData(0x0A);
	BSP_LCD_WriteData(0xA2);

	/* Power control 1 (C0h) */
	BSP_LCD_WriteCmd(0xC0);
	BSP_LCD_WriteData(0x35);

	/* Power control 2 (C1h) */
	BSP_LCD_WriteCmd(0xC1);
	BSP_LCD_WriteData(0x11);

	/* VCOM control 1 (C5h) */
	BSP_LCD_WriteCmd(0xC5);
	BSP_LCD_WriteData(0x45);
	BSP_LCD_WriteData(0x45);

	/* VCOM control 2 (C7h) */
	BSP_LCD_WriteCmd(0xC7);
	BSP_LCD_WriteData(0xA2);

	/* Enable 3G (F2h) */
	BSP_LCD_WriteCmd(0xF2);
	BSP_LCD_WriteData(0x00);

	/* Gamma set (26h) */
	BSP_LCD_WriteCmd(0x26);
	BSP_LCD_WriteData(0x01);

	/* Positive gamma correction (E0h) */
	BSP_LCD_WriteCmd(0xE0);
	BSP_LCD_WriteData(0x0F);
	BSP_LCD_WriteData(0x26);
	BSP_LCD_WriteData(0x24);
	BSP_LCD_WriteData(0x0B);
	BSP_LCD_WriteData(0x0E);
	BSP_LCD_WriteData(0x09);
	BSP_LCD_WriteData(0x54);
	BSP_LCD_WriteData(0xA8);
	BSP_LCD_WriteData(0x46);
	BSP_LCD_WriteData(0x0C);
	BSP_LCD_WriteData(0x17);
	BSP_LCD_WriteData(0x09);
	BSP_LCD_WriteData(0x0F);
	BSP_LCD_WriteData(0x07);
	BSP_LCD_WriteData(0x00);

	/* Negative gamma correction (E1h) */
	BSP_LCD_WriteCmd(0xE1);
	BSP_LCD_WriteData(0x00);
	BSP_LCD_WriteData(0x19);
	BSP_LCD_WriteData(0x1B);
	BSP_LCD_WriteData(0x04);
	BSP_LCD_WriteData(0x10);
	BSP_LCD_WriteData(0x07);
	BSP_LCD_WriteData(0x2A);
	BSP_LCD_WriteData(0x47);
	BSP_LCD_WriteData(0x39);
	BSP_LCD_WriteData(0x03);
	BSP_LCD_WriteData(0x06);
	BSP_LCD_WriteData(0x06);
	BSP_LCD_WriteData(0x30);
	BSP_LCD_WriteData(0x38);
	BSP_LCD_WriteData(0x0F);

	/* Memory access control (36h) */
	/* 0xC8:竖屏，左上角为起点到右下角扫描 */
	BSP_LCD_WriteCmd(0x36);
	BSP_LCD_WriteData(0xC8);

	/* Column address set (2Ah) */
	BSP_LCD_WriteCmd(LCD_SET_COL_CMD);
	BSP_LCD_WriteData(0x00);
	BSP_LCD_WriteData(0x00);
	BSP_LCD_WriteData(0x00);
	BSP_LCD_WriteData(0xEF);

	/* Page address set (2Bh) */
	BSP_LCD_WriteCmd(LCD_SET_PAGE_CMD);
	BSP_LCD_WriteData(0x00);
	BSP_LCD_WriteData(0x00);
	BSP_LCD_WriteData(0x01);
	BSP_LCD_WriteData(0x3F);

	/* Pixel format set (3Ah) */
	/* 0x55=16bit RGB565 */
	BSP_LCD_WriteCmd(0x3A);
	BSP_LCD_WriteData(0x55);

	/* Sleep out (11h) */
	BSP_LCD_WriteCmd(0x11);
	BSP_Delay_ms(120); /* 延时 120ms 等待 LCD 退出休眠 */

	/* Display ON (29h) */
	BSP_LCD_WriteCmd(0x29);
	}
}

static void BSP_LCD_FSMC_Config(void)
{
	const bsp_lcd_hw_t *hw = &bsp_lcd_hw;
	FSMC_NORSRAMInitTypeDef FSMC_InitStruct;
	FSMC_NORSRAMTimingInitTypeDef readTiming;
	FSMC_NORSRAMTimingInitTypeDef writeTiming;

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

void BSP_LCD_Init(void)
{
    /* 1. 初始化液晶相关的普通 GPIO 引脚并复位 LCD */
    BSP_LCD_Private_Pins_Init();
    
    /* 2. 配置 FSMC 读写时序及总线 */
    BSP_LCD_FSMC_Config();
    /* 3. FSMC通信配置完成后，通过对屏幕写入命令配置 LCD 的核心寄存器 */
    BSP_LCD_REG_Config();
}

void BSP_LCD_Test_Demo(void)
{
    /* 1. 初始化屏幕外设硬件与寄存器 */
    BSP_LCD_Init();
    
    /* 2. 在开始画图之前，先将整个屏幕填充为黑色背景，清空杂乱数据 */
    BSP_LCD_Clear(BLACK);
    
    /* 
     * 3. 测试画点: BSP_LCD_DrawPixel(x, y, 颜色)
     * 在屏幕坐标 (10, 10) 的位置画一个红色的点 
     */
    BSP_LCD_DrawPixel(10, 10, RED);
    
    /* 
     * 4. 测试画线: BSP_LCD_DrawLine(起点x, 起点y, 终点x, 终点y, 颜色)
     * 从坐标 (20, 20) 到 (100, 100) 画一条绿色的斜线
     */
    BSP_LCD_DrawLine(20, 20, 100, 100, GREEN);
    
    /* 水平线测试：从 (20, 120) 到 (100, 120) 画一条蓝色的横线 */
    BSP_LCD_DrawLine(20, 120, 100, 120, BLUE);
    
    /* 
     * 5. 测试画矩形边框: BSP_LCD_DrawRect(左上角x, 左上角y, 宽度, 高度, 颜色)
     * 在坐标 (120, 20) 处画一个宽 60、高 80 的黄色矩形框
     */
    BSP_LCD_DrawRect(120, 20, 60, 80, YELLOW);
    
    /* 
     * 6. 测试画圆: BSP_LCD_DrawCircle(圆心x, 圆心y, 半径, 颜色)
     * 在坐标 (120, 180) 处，画一个半径为 40 的红色圆形
     */
    BSP_LCD_DrawCircle(120, 180, 40, RED);
}