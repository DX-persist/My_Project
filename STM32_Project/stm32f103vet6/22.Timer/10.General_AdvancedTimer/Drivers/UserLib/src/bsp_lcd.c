#include "bsp_lcd.h"
#include "bsp_gpio.h"

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
	bsp_gpio_t lcd_reset;
	bsp_gpio_t lcd_bl;
	bsp_gpio_t lcd_cs;
	bsp_gpio_t lcd_dc;

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

/* 定义全局的宽高变量，默认值对应竖屏 */
uint16_t lcd_width  = 240;
uint16_t lcd_height = 320;
uint8_t  lcd_scan_mode = 6; /* 默认扫描方向:模式6 */

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

/**
 * @brief  设置液晶屏的 GRAM 扫描方向 (改变横竖屏模式)
 * @param  option: 0-7 共 8 个方向
 * 
 *	 ！！注意：0、3、5、6 模式适合从左至右显示文字，其它引出镜像效果
 *     其中 0 2 4 6 模式为竖屏，宽 240，高 320 (推荐模式 6)
 *     其中 1 3 5 7 模式为横屏，宽 320，高 240 (推荐模式 3)
 */
void BSP_LCD_SetScanDir(uint8_t option)
{
    if (option > 7) return;

    /* 1. 更新全局的扫描模式和宽高变量 */
    lcd_scan_mode = option;

    if (option % 2 == 0) {
        /* 0 2 4 6 模式为竖屏 */
        lcd_width  = 240;
        lcd_height = 320;
    } else {
        /* 1 3 5 7 模式为横屏 */
        lcd_width  = 320;
        lcd_height = 240;
    }

    /* 2. 发送 0x36 控制指令改变扫描方向
       高三位(位5,6,7)由 option 的值决定，0x08 代表 BGR 颜色格式排布 */
    BSP_LCD_WriteCmd(0x36);
    BSP_LCD_WriteData(0x08 | (option << 5));

    /* 3. 因为画幅宽高改变了，必须重新开辟一次全屏窗口 */
    BSP_LCD_SetWindow(0, 0, lcd_width - 1, lcd_height - 1);

    /* 4. 准备写入显存 */
    BSP_LCD_WriteCmd(LCD_SET_MEM_CMD);
}

/* ------------------- 对外暴露的绘图接口 ------------------- */
/**
 * @brief  在指定像素点画一个带有颜色的点
 */
void BSP_LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if(x >= lcd_width || y >= lcd_height) return;
    
    BSP_LCD_SetCursor(x, y);
    BSP_LCD_WriteCmd(LCD_SET_MEM_CMD);      
    BSP_LCD_WriteData(color);    
}
/**
 * @brief  以特定的颜色清空整个屏幕
 */
void BSP_LCD_Clear(uint16_t color)
{
    uint32_t i = 0;
    uint32_t total_points = (uint32_t)lcd_width * lcd_height;
    
    BSP_LCD_SetWindow(0, 0, lcd_width - 1, lcd_height - 1);
    
    BSP_LCD_WriteCmd(LCD_SET_MEM_CMD); 
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
 * @brief  画矩形 / 实心矩形
 */
void BSP_LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t filled)
{
    if (filled) {
        /* 如果是实心，借用设置窗口的方式极速填充 */
        BSP_LCD_SetWindow(x, y, x + width - 1, y + height - 1);
        BSP_LCD_WriteCmd(LCD_SET_MEM_CMD);
        uint32_t total_points = (uint32_t)width * height;
        for (uint32_t i = 0; i < total_points; i++) {
            BSP_LCD_WriteData(color);
        }
    } else {
        /* 仅绘制边框 */
        BSP_LCD_DrawLine(x, y, x + width - 1, y, color);                           /* 上边 */
        BSP_LCD_DrawLine(x, y + height - 1, x + width - 1, y + height - 1, color); /* 下边 */
        BSP_LCD_DrawLine(x, y, x, y + height - 1, color);                          /* 左边 */
        BSP_LCD_DrawLine(x + width - 1, y, x + width - 1, y + height - 1, color);  /* 右边 */
    }
}
/**
 * @brief  画圆 / 实心圆 (基于 Bresenham 算法)
 */
void BSP_LCD_DrawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color, uint8_t filled)
{
    int a = 0, b = r;
    int di = 3 - (r << 1); /* 判断下个点位置的标志 */
    
    while (a <= b) {
        if (filled) {
            /* 填充圆：通过画水平线连接左右对称点进行逐行填充 */
            BSP_LCD_DrawLine(x0 - a, y0 - b, x0 + a, y0 - b, color);
            BSP_LCD_DrawLine(x0 - a, y0 + b, x0 + a, y0 + b, color);
            BSP_LCD_DrawLine(x0 - b, y0 - a, x0 + b, y0 - a, color);
            BSP_LCD_DrawLine(x0 - b, y0 + a, x0 + b, y0 + a, color);
        } else {
            /* 仅绘制圆弧轮廓 */
            BSP_LCD_DrawPixel(x0 + a, y0 - b, color); /* 5 区 */
            BSP_LCD_DrawPixel(x0 + b, y0 - a, color); /* 0 区 */
            BSP_LCD_DrawPixel(x0 + b, y0 + a, color); /* 7 区 */
            BSP_LCD_DrawPixel(x0 + a, y0 + b, color); /* 6 区 */
            BSP_LCD_DrawPixel(x0 - a, y0 + b, color); /* 1 区 */
            BSP_LCD_DrawPixel(x0 - b, y0 + a, color); /* 2 区 */
            BSP_LCD_DrawPixel(x0 - a, y0 - b, color); /* 4 区 */
            BSP_LCD_DrawPixel(x0 - b, y0 - a, color); /* 3 区 */
        }
        
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
	BSP_Delay_ms(120);
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

	BSP_LCD_SetScanDir(6);
	
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
     * 5. 测试画矩形边框: BSP_LCD_DrawRect(左上角x, 左上角y, 宽度, 高度, 颜色, 填充标志)
     * 在坐标 (120, 20) 处画一个宽 60、高 80 的黄色矩形框，0代表不填充
     */
    BSP_LCD_DrawRect(105, 20, 60, 80, YELLOW, 0);

    /* 在坐标 (200, 20) 处测试一个填充的紫色矩形 */
    BSP_LCD_DrawRect(170, 20, 60, 80, PURPLE, 1);
    
    /* 
     * 6. 测试画圆: BSP_LCD_DrawCircle(圆心x, 圆心y, 半径, 颜色, 填充标志)
     * 在坐标 (120, 180) 处，画一个半径为 40 的红色圆形，0代表不填充
     */
    BSP_LCD_DrawCircle(100, 180, 40, RED, 0);

    /* 在坐标 (200, 180) 处，测试一个半径为 40 的实心蓝色圆形 */
    BSP_LCD_DrawCircle(190, 180, 40, BLUE, 1);
}

/**
 * @brief      在LCD上显示自定义大小的字符
 * @param      x        字符显示的起始 X 坐标
 * @param      y        字符显示的起始 Y 坐标
 * @param      font     字模数据的首地址
 * @param      width    字符宽度像素 (如: 8、16、24)
 * @param      height   字符高度像素 (如: 16、24、32)
 * @param      fg_color 前景色 (字体的颜色)
 * @param      bg_color 背景色 (空白处的颜色)
 */
void BSP_LCD_DrawChar(uint16_t x, uint16_t y, const uint8_t *font, uint8_t width, uint8_t height, uint16_t fg_color, uint16_t bg_color)
{
    // 1. 根据宽度，计算该字模每一行需要读取多少个字节
    uint8_t bytes_per_row = (width + 7) / 8;

    // 2. 设置绘图窗口，限制液晶屏自动填充数据时光标的移动范围
    BSP_LCD_SetWindow(x, y, x + width - 1, y + height - 1);
    
    // 3. 【重要】发送写显存命令，告诉硬件准备接收像素颜色！
    BSP_LCD_WriteCmd(LCD_SET_MEM_CMD);

    // 4. 开始逐行逐列向屏幕写入像素颜色
    for (uint16_t row = 0; row < height; row++) {
        for (uint16_t col = 0; col < width; col++) {
            
            // 计算当前列对应的像素，存在这一行的第几个字节里
            uint8_t byte_idx = row * bytes_per_row + (col / 8);
            uint8_t byte_val = font[byte_idx];
            
            // 计算当前像素属于该字节的第几个比特位 
            // （普通的字模由左至右读取，最高位 bit7 对应最左侧的像素）
            uint8_t bit_shift = 7 - (col % 8);
            
            // 提取该比特位的值，如果是 1 则画前景色，0 画背景色
            uint8_t pixel = (byte_val >> bit_shift) & 0x01;
            
            // 将点写入 LCD 显存
            BSP_LCD_WriteData(pixel ? fg_color : bg_color);
        }
    }
}
