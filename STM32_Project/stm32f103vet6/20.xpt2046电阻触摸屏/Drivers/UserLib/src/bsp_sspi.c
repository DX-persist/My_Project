#include "bsp_sspi.h"

/**
 * @brief 软件SPI引脚配置结构
 *
 * 保存软件SPI使用的GPIO引脚信息
 */
typedef struct{
	bsp_gpio_t sspi_cs;    /**< 片选引脚 */
	bsp_gpio_t sspi_mosi;  /**< 主输出从输入 */
	bsp_gpio_t sspi_miso;  /**< 主输入从输出 */
	bsp_gpio_t sspi_clk;   /**< SPI时钟 */
}bsp_spi_sw_t;


/**
 * @brief 软件SPI引脚配置表
 *
 * 每个软件SPI实例对应一组GPIO引脚
 */
static const bsp_spi_sw_t bsp_spi_sw[BSP_SSPI_MAX] = {
	[BSP_SSPI1] = {
		.sspi_cs 	= {GPIOD, GPIO_Pin_13, RCC_APB2Periph_GPIOD},
		.sspi_mosi 	= {GPIOE, GPIO_Pin_2, RCC_APB2Periph_GPIOE},
		.sspi_miso 	= {GPIOE, GPIO_Pin_3, RCC_APB2Periph_GPIOE},
		.sspi_clk  	= {GPIOE, GPIO_Pin_0, RCC_APB2Periph_GPIOE},
	},
};


/**
 * @brief 软件SPI时序延时
 *
 * 用于控制SPI通信速率
 *
 * @note
 * 可根据系统主频调整循环次数
 */
static inline void BSP_SSPI_Delay(void)
{
	for(volatile uint8_t i = 0; i < 10; i++){
		__NOP();
	}
}


/**
 * @brief 初始化软件SPI
 *
 * 初始化SPI相关GPIO：
 * - CS  输出
 * - MOSI 输出
 * - CLK 输出
 * - MISO 输入
 *
 * SPI工作模式：
 * Mode0 (CPOL=0, CPHA=0)
 *
 * @param sspi_id 软件SPI编号
 */
void BSP_SSPI_Init(bsp_sspi_t sspi_id)
{
	if(sspi_id >= BSP_SSPI_MAX)	return;
	
	const bsp_spi_sw_t *sw = &bsp_spi_sw[sspi_id];
	uint32_t enable_clk = 0;
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);

	/** 开启GPIO时钟 */
	enable_clk = sw->sspi_cs.rcc_clk | sw->sspi_mosi.rcc_clk |
					sw->sspi_miso.rcc_clk | sw->sspi_clk.rcc_clk;
	RCC_APB2PeriphClockCmd(enable_clk, ENABLE);

	/** 初始化GPIO默认电平 */
	BSP_GPIO_WritePin(&sw->sspi_cs, Bit_SET);      /**< 默认不选中 */
	BSP_GPIO_WritePin(&sw->sspi_clk, Bit_RESET);   /**< SPI Mode0 时钟空闲低 */
	BSP_GPIO_WritePin(&sw->sspi_mosi, Bit_RESET);	
	
	/** 配置CS、MOSI、CLK为推挽输出 */
	GPIO_InitStruct.GPIO_Pin = sw->sspi_cs.gpio_pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(sw->sspi_cs.gpio_port, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = sw->sspi_mosi.gpio_pin;
	GPIO_Init(sw->sspi_mosi.gpio_port, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = sw->sspi_clk.gpio_pin;
	GPIO_Init(sw->sspi_clk.gpio_port, &GPIO_InitStruct);

	/** 配置MISO为输入 */
	GPIO_InitStruct.GPIO_Pin = sw->sspi_miso.gpio_pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(sw->sspi_miso.gpio_port, &GPIO_InitStruct);
}


/**
 * @brief 控制SPI片选
 *
 * @param sspi_id 软件SPI编号
 * @param option  片选状态
 */
void BSP_SSPI_CtlChip(bsp_sspi_t sspi_id, uint8_t option)
{
	if(sspi_id >= BSP_SSPI_MAX)	return;

	const bsp_spi_sw_t *sw = &bsp_spi_sw[sspi_id];
	
	BSP_GPIO_WritePin(&sw->sspi_cs, option);
}


/**
 * @brief SPI读写一个字节
 *
 * 使用软件方式模拟SPI通信。
 * 每发送一位数据，同时读取一位数据。
 *
 * SPI模式：
 * Mode0
 * - CPOL = 0
 * - CPHA = 0
 *
 * 时序：
 * 1. MOSI准备数据
 * 2. CLK上升沿
 * 3. 读取MISO
 * 4. CLK下降沿
 *
 * @param sspi_id   软件SPI编号
 * @param writebyte 发送字节
 * @param readbyte  接收数据指针
 *
 * @warning readbyte 可以为NULL
 */
void BSP_SSPI_ReadWriteByte(bsp_sspi_t sspi_id, uint8_t writebyte, uint8_t *readbyte)
{
	if(sspi_id >= BSP_SSPI_MAX)	return;
	
	const bsp_spi_sw_t *sw = &bsp_spi_sw[sspi_id];
	uint8_t temp_data = 0;

	for(int i = 0; i < 8; i++){

		/** 发送最高位 */
		if(writebyte & 0x80){
			BSP_GPIO_WritePin(&sw->sspi_mosi, Bit_SET);
		}else{
			BSP_GPIO_WritePin(&sw->sspi_mosi, Bit_RESET);
		}

		writebyte <<= 1;

		BSP_SSPI_Delay();

		/** 时钟上升沿 */
		BSP_GPIO_WritePin(&sw->sspi_clk, Bit_SET);

		BSP_SSPI_Delay();
		
		/** 读取MISO */
		temp_data <<= 1;
		if(BSP_GPIO_ReadPin(&sw->sspi_miso)){
			temp_data |= 0x01;
		}

		/** 时钟下降沿 */
		BSP_GPIO_WritePin(&sw->sspi_clk, Bit_RESET);

		BSP_SSPI_Delay();
	}

	/** 返回接收数据 */
	if(readbyte != NULL){
		*readbyte = temp_data;
	}
}