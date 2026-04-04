/**
 * @file    bsp_hspi.c
 * @author  Antigravity
 * @brief   硬件 SPI 外设底层驱动实现文件
 * @version V1.0
 * @note    本文件实现了 SPI1/SPI2/SPI3 的硬件初始化配置及基础的数据收发接口。
 *          通过维护一个统一的硬件配置参数表 (bsp_spi_hw_t)，提高了代码的可移植性和解耦度。
 */

#include "bsp_hspi.h"

/**
 * @brief 硬件 SPI 底层配置信息结构体
 * @note  包含 SPI 外设的所有相关参数、引脚配置及所在总线信息，便于进行多通道统一管理。
 */
typedef struct{
	SPI_TypeDef *spi;       /*!< SPI 外设基地址 (如 SPI1, SPI2 等) */
	uint32_t spi_clk;       /*!< SPI 外设时钟门控宏 */

	uint16_t direction;     /*!< 数据传输方向模式 (双线全双工等) */
	uint16_t mode;          /*!< 主从模式 (主模式/从模式) */
	uint16_t datasize;      /*!< 数据帧大小 (8位/16位) */
	uint16_t cpol;          /*!< 时钟极性 (空闲时是高还是低) */
	uint16_t cpha;          /*!< 时钟相位 (第一还是第二跳变沿采样) */
	uint16_t nss;           /*!< NSS 片选引脚的控制方式 (软件/硬件管理) */
	uint16_t baud_pre;      /*!< 波特率预分频值 */
	uint16_t first_bit;     /*!< 数据传输高位先行还是低位先行 */
	uint16_t crc_pol;       /*!< CRC 校验多项式 (若不使用填 0 即可) */

	GPIO_TypeDef *sck_port; /*!< SCK 引脚所在 GPIO 端口 */
	uint16_t sck_pin;       /*!< SCK 引脚编号 */
	uint32_t sck_clk;       /*!< SCK 引脚外设时钟宏 */

	GPIO_TypeDef *mosi_port;/*!< MOSI 引脚所在 GPIO 端口 */
	uint16_t mosi_pin;      /*!< MOSI 引脚编号 */
	uint32_t mosi_clk;      /*!< MOSI 引脚外设时钟宏 */

	GPIO_TypeDef *miso_port;/*!< MISO 引脚所在 GPIO 端口 */
	uint16_t miso_pin;      /*!< MISO 引脚编号 */
	uint32_t miso_clk;      /*!< MISO 引脚外设时钟宏 */

	BSP_BusTypeDef bus;     /*!< SPI 外设所挂载的系统总线枚举 (APB1 或 APB2) */
}bsp_spi_hw_t;

/** @brief 硬件 SPI 引脚配置及外设参数常量表 */
static const bsp_spi_hw_t bsp_hspi_hw[BSP_HSPI_MAX] = {
	[BSP_HSPI1] = {
		.spi 		= SPI1,
		.spi_clk 	= RCC_APB2Periph_SPI1,

		.direction 	= SPI_Direction_2Lines_FullDuplex,
		.mode 		= SPI_Mode_Master,
		.datasize 	= SPI_DataSize_8b,
		.cpol 		= SPI_CPOL_High,
		.cpha 		= SPI_CPHA_2Edge,
		.nss 		= SPI_NSS_Soft,
		.baud_pre 	= SPI_BaudRatePrescaler_2,
		.first_bit 	= SPI_FirstBit_MSB,
		.crc_pol 	= 0,
		
		.sck_port	= GPIOA,
		.sck_pin	= GPIO_Pin_5,
		.sck_clk	= RCC_APB2Periph_GPIOA,

		.mosi_port	= GPIOA,
		.mosi_pin	= GPIO_Pin_7,
		.mosi_clk	= RCC_APB2Periph_GPIOA,

		.miso_port	= GPIOA,
		.miso_pin	= GPIO_Pin_6,
		.miso_clk	= RCC_APB2Periph_GPIOA,

		.bus		= BSP_BUS_APB2,
	},

	[BSP_HSPI2] = {
		.spi 		= SPI2,
		.spi_clk 	= RCC_APB1Periph_SPI2,

		.direction 	= SPI_Direction_2Lines_FullDuplex,
		.mode 		= SPI_Mode_Master,
		.datasize 	= SPI_DataSize_8b,
		.cpol 		= SPI_CPOL_High,
		.cpha 		= SPI_CPHA_2Edge,
		.nss 		= SPI_NSS_Soft,
		.baud_pre 	= SPI_BaudRatePrescaler_2,
		.first_bit 	= SPI_FirstBit_MSB,
		.crc_pol 	= 0,
		
		.sck_port	= GPIOB,
		.sck_pin	= GPIO_Pin_13,
		.sck_clk	= RCC_APB2Periph_GPIOB,

		.mosi_port	= GPIOB,
		.mosi_pin	= GPIO_Pin_15,
		.mosi_clk	= RCC_APB2Periph_GPIOB,

		.miso_port	= GPIOB,
		.miso_pin	= GPIO_Pin_14,
		.miso_clk	= RCC_APB2Periph_GPIOB,

		.bus		= BSP_BUS_APB1,
	},

	[BSP_HSPI3] = {
		.spi 		= SPI3,
		.spi_clk 	= RCC_APB1Periph_SPI3,

		.direction 	= SPI_Direction_2Lines_FullDuplex,
		.mode 		= SPI_Mode_Master,
		.datasize 	= SPI_DataSize_8b,
		.cpol 		= SPI_CPOL_High,
		.cpha 		= SPI_CPHA_2Edge,
		.nss 		= SPI_NSS_Soft,
		.baud_pre 	= SPI_BaudRatePrescaler_2,
		.first_bit 	= SPI_FirstBit_MSB,
		.crc_pol 	= 0,
		
		.sck_port	= GPIOB,
		.sck_pin	= GPIO_Pin_3,
		.sck_clk	= RCC_APB2Periph_GPIOB,

		.mosi_port	= GPIOB,
		.mosi_pin	= GPIO_Pin_5,
		.mosi_clk	= RCC_APB2Periph_GPIOB,

		.miso_port	= GPIOB,
		.miso_pin	= GPIO_Pin_4,
		.miso_clk	= RCC_APB2Periph_GPIOB,

		.bus		= BSP_BUS_APB1,
	}
};

/**
 * @brief  初始化指定的硬件 SPI 外设及其对应的 GPIO 引脚
 * @param  hspi_id   SPI 通道号 (BSP_HSPI1 / BSP_HSPI2 / BSP_HSPI3)
 * @retval 无
 * @note   包含对应 GPIO 引脚的复用功能配置 (SCK, MOSI 配置为复用推挽输出，MISO 配置为浮空输入)。
 *         注意：在使用 SPI3 时会由于引脚复用冲突而自动禁用 JTAG 功能（保留 SWD 下载和调试）。
 */
void BSP_HSPI_Init(bsp_hspi_t hspi_id)
{
	if(hspi_id >= BSP_HSPI_MAX)	return;

	const bsp_spi_hw_t *hw = &bsp_hspi_hw[hspi_id];
	GPIO_InitTypeDef GPIO_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;

	/* 开启 GPIO 使用的时钟 */
	RCC_APB2PeriphClockCmd(hw->sck_clk | hw->miso_clk | hw->mosi_clk, ENABLE);

	/* 开启 SPI 使用的时钟 */
	if(hw->bus == BSP_BUS_APB1){
		RCC_APB1PeriphClockCmd(hw->spi_clk, ENABLE);
	}else{
		RCC_APB2PeriphClockCmd(hw->spi_clk, ENABLE);
	}

	/* 由于 SPI3 的NSS SCK MISO 引脚和JTAG 接口冲突，
	 * 所以这里禁用JTAG接口，但stlink并不影响下载 */
	if(hspi_id == BSP_HSPI3){
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	}

	/* 初始化 SPI 使用的SCK MISO MOSI 引脚 */
	GPIO_StructInit(&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin 	= hw->sck_pin;
	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(hw->sck_port, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin 	= hw->miso_pin;
	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
	GPIO_Init(hw->miso_port, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin 	= hw->mosi_pin;
	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(hw->mosi_port, &GPIO_InitStruct);

	/* 初始化 SPI 结构体的成员 */
	SPI_StructInit(&SPI_InitStruct);

	SPI_InitStruct.SPI_Direction 			= hw->direction;
	SPI_InitStruct.SPI_Mode					= hw->mode;
	SPI_InitStruct.SPI_DataSize				= hw->datasize;
	SPI_InitStruct.SPI_CPOL					= hw->cpol;
	SPI_InitStruct.SPI_CPHA					= hw->cpha;
	SPI_InitStruct.SPI_NSS					= hw->nss;
	SPI_InitStruct.SPI_BaudRatePrescaler 	= hw->baud_pre;
	SPI_InitStruct.SPI_FirstBit				= hw->first_bit;
	SPI_InitStruct.SPI_CRCPolynomial 		= hw->crc_pol;
	SPI_Init(hw->spi, &SPI_InitStruct);

	/* 启用SPI */
	SPI_Cmd(hw->spi, ENABLE);
}

/**
 * @brief  通过硬件 SPI 总线发送并接收一个字节数据（同步双工传输）
 * @param  hspi_id    SPI 通道号
 * @param  write_byte 要发送给目标从机的单字节数据
 * @param  read_byte  用于接收从机返回响应的数据指针 (可传入 NULL 忽略接收)
 * @retval 错误码，返回 HSPI_OK (0) 表示收发成功，其他值代表超时异常（如 TXE/RXNE 等待失败）
 */
hspi_status_t BSP_HSPI_ReadWriteByte(bsp_hspi_t hspi_id, uint8_t write_byte, uint8_t *read_byte)
{
	if(hspi_id >= BSP_HSPI_MAX)	return HSPI_ERROR_PARA;

	uint16_t timeout = BSP_HSPI_TIMEOUT_MAX;
	const bsp_spi_hw_t *hw = &bsp_hspi_hw[hspi_id];

	/* 判断发送数据寄存器是否为空 */
	timeout = BSP_HSPI_TIMEOUT_MAX;
	while(SPI_I2S_GetFlagStatus(hw->spi, SPI_I2S_FLAG_TXE) == RESET){
		if((timeout--) == 0){
			return HSPI_ERROR_TXE;
		}
	}

	/* 发送数据寄存器为空可以写入数据 */
	SPI_I2S_SendData(hw->spi, (uint16_t)write_byte);

	/* 判断接受数据寄存器是否非空 
	 * 由于 SPI 是同步通信，MOSI 发送一位， MISO 就会接收一位，
	 * 理论上当一个字节传送完(假设这里是8个数据位),也就接收到
	 * 8个数据位并且存放到 DR 寄存器中，此时 RXNE 标志位置为1
	 * 所以判断 RXNE 标志位是否为1等同于判断该次数据流程发送
	 * 是否结束
	 *
	 * 并且这里需要注意的是由于 SPI 是同步通信，它接收到的数据
	 * 会存放到 DR 寄存器中，若不读取等到下一个数据来的时候就会
	 * 产生移除，此时 OVR 标志位会被置为1，所以当检测到 RXNE 标
	 * 志位置为1的时候需要将其数据读取出来
	 * */
	timeout = BSP_HSPI_TIMEOUT_MAX;
	while(SPI_I2S_GetFlagStatus(hw->spi, SPI_I2S_FLAG_RXNE) == RESET){
		if((timeout--) == 0){
			return HSPI_ERROR_RXNE;
		}
	}
	/* 这里使用临时变量来存放数据，然后将数据赋值给read_byte防止上层
	   传入一个空指针导致对空指针操作引起硬件报错
	   不管上层需不需要都要读取 DR 寄存器的数据以此来清空 RXNE 标志位
	*/
	uint8_t temp = (uint8_t)SPI_I2S_ReceiveData(hw->spi);
	if(read_byte != NULL){
		*read_byte = temp;
	}

	return HSPI_OK;
}

/**
 * @brief  轮询等待指定的硬件 SPI 外设总线所有当前传输任务完成并进入空闲状态
 * @param  hspi_id   SPI 通道号
 * @retval 错误码，返回 HSPI_OK 表示总线已空闲，返回 HSPI_ERROR_BUSY 表示等待硬件空闲超时
 */
hspi_status_t BSP_HSPI_WaitIdle(bsp_hspi_t hspi_id)
{
	if(hspi_id >= BSP_HSPI_MAX)	return HSPI_ERROR_PARA;

	const bsp_spi_hw_t *hw = &bsp_hspi_hw[hspi_id];
	uint16_t timeout = BSP_HSPI_TIMEOUT_MAX;

	while(SPI_I2S_GetFlagStatus(hw->spi, SPI_I2S_FLAG_BSY) == SET){
		if((timeout--) == 0){
			return HSPI_ERROR_BUSY;
		}
	}
	return HSPI_OK;
}
