#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_fatfs.h"
#include "ff.h"

#include <stdio.h>
#include <string.h>  /* 使用 strlen 需包含 */

FATFS spi_flash_fs;	            /**< 文件系统对象 */
BYTE format_work_buf[FF_MAX_SS]; /**< f_mkfs 格式化工作缓冲区 */
FIL fp;                         /**< 文件对象 */

/**
 * @brief  主函数：FatFs 文件系统的串口交互与读写测试流程
 * @retval int
 */
int main(void)
{
	/* 因为不使用旧代码所以去掉了未使用的 ret 变量 */
	BYTE write_buff[] = "这是一个测试Flash写入字符串";
	BYTE read_buff[64] = {0};  /* 建议初始化清零 */
	BYTE file_string[32] = {0};
	BYTE file_name[64] = {0};

	UINT byte_written = 0;
	UINT byte_read = 0;

	BSP_LED_Init();
	BSP_TimeBase_Init();
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Init_RxBuffer(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);

	FATFS_LOG("%s|%s|%d 串行Flash文件系统测试\r\n", __FILE__, __func__, __LINE__);
	FATFS_LOG("%s|%s|%d 请输入文件名(例如: test.txt)\r\n", __FILE__, __func__, __LINE__);
	scanf("%s",file_string);
	printf("%s|%s|%d 输入的文件名是: %s\r\n", __FILE__, __func__, __LINE__, file_string);
	
	/**< 将串口获取的文件名和逻辑设备驱动号(1:)联系起来 */
	sprintf((char *)file_name, "1:%s", file_string);
	printf("%s|%s|%d 关联后的文件名是: %s\r\n", __FILE__, __func__, __LINE__, file_name);

	/* 初始化并挂载文件系统 */
	BSP_FatFs_Init(&spi_flash_fs, format_work_buf);

	/* 打开/创建文件 */
	BSP_Fatfs_OpenFile(&fp, (const TCHAR*)file_name);
	
	/* 写入字符串数据：使用 sizeof() - 1 剔除末尾的 '\0' 结束符，
	 * 保证生成的文档是纯文本文件而不会被编辑器识别为二进制文件
	 */
	BSP_Fatfs_WriteData(&fp, write_buff, sizeof(write_buff) - 1, &byte_written);
	FATFS_LOG("%s|%s|%d 写入的数据为:[%s]\r\n",__FILE__, __func__, __LINE__, (char *)write_buff);
	
	/* 读取字符串数据：根据待写入字符在内存长度来读取对应的字节数 */
	BSP_Fatfs_ReadData(&fp, read_buff, strlen((char *)write_buff), &byte_read);
	
	/* 为保证乱码安全，手动补一个结束符（虽然前面初始化过）*/
	read_buff[byte_read] = '\0';
	FATFS_LOG("%s|%s|%d 读取到的数据为:[%s]\r\n",__FILE__, __func__,__LINE__, (char *)read_buff);
	
	/* 关闭并保存文件 */
	BSP_Fatfs_CloseFile(&fp);

	while(1){
	
	}
}
