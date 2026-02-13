#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_dma.h"

#include <stdio.h>

#define BUFFER_SIZE	8

uint8_t CompareBuffer(const uint32_t *flash_buffer, uint32_t *sram_buffer, uint8_t len)
{
	for(int i = 0; i < len; i++){
		if(flash_buffer[i] != sram_buffer[i]){
			return 0;
		}
	}
	return 1;
}

int main(void)
{
	const uint32_t flash_buffer[BUFFER_SIZE] = {0x1234, 0x5678, 0x9012, 0x3456, 0x7890, 0x1357, 0x2468, 0x1481};
	uint32_t sram_buffer[BUFFER_SIZE];

	bsp_dma_config_t config = {
		.periph_addr = (uint32_t)flash_buffer,
		.memory_addr = (uint32_t)sram_buffer,
		.dir = DIR_Periph_SRC,
		.buffer_size = BUFFER_SIZE,
		.periph_inc = PeripheralInc_Enable,
		.memory_inc = MemoryInc_Enable,
		.periph_data_size = PeripheralDataSize_Word,
		.memory_data_size = MemoryDataSize_Word,
		.mode = DMA_Mode_Nor,
		.priority = DMA_Priority_M,
		.m2m = DMA_M2M_ENABLE,
	};

    BSP_LED_Init();
	BSP_TimeBase_Init();
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);
	BSP_DMA_Init(BSP_DMA1_Channel1, &config);
			
	while(DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET);

	if(CompareBuffer(flash_buffer, sram_buffer, BUFFER_SIZE)){
		BSP_LED_On(LED_BLUE);
		for(int i = 0; i < BUFFER_SIZE; i++){
			printf("0x%08x\r\n",sram_buffer[i]);
		}
	}else{
		BSP_LED_On(LED_RED);
	}
	while(1){
	
	}

}
