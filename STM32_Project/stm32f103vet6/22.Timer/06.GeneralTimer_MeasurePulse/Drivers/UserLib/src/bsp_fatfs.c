#include "bsp_fatfs.h"

/**
 * @brief  初始化 FatFs 文件系统并挂载指定的存储设备
 * @param  fs   指向文件系统对象 (FATFS) 的指针。如果不使用多个文件系统, 则可以直接传入 NULL (取决于具体的底层实现)
 * @param  buff 指向为 f_mkfs 格式化时提供的工作缓冲区的指针，大小至少为 FF_MAX_SS
 * @note   f_mount 函数注册文件系统对象，在使用 f_open 等函数前必须调用此函数初始化：
 *         1. 参数 path: 逻辑驱动号路径，格式为"0:"，其中0为逻辑驱动号，可以根据需要修改为"1:"等
 *         2. 参数 opt: 是否立即挂载文件系统，如果为0则延迟挂载只有当后边调用 f_open 等函数时才挂载；
 *            如果为1才会立即挂载并验证文件系统，如果挂载失败则返回错误码。
 *         如果挂载失败且错误码为 FR_NO_FILESYSTEM，本函数会自动调用 f_mkfs 进行格式化。
 * @retval 无
 */
void BSP_FatFs_Init(FATFS *fs, BYTE *buff)
{
	FRESULT ret;
	
	ret = f_mount(fs, "1:", 1);
	if(ret == FR_OK){
		FATFS_LOG("%s|%s|%d W25Q64 挂载成功\r\n",__FILE__, __func__, __LINE__);
	}else if(ret == FR_NO_FILESYSTEM){
		FATFS_LOG("%s|%s|%d W25Q64 挂载失败,该存储介质未格式化且并未存在文件系统\r\n",__FILE__, __func__, __LINE__);
		
		/* 调用 f_mkfs 函数格式化存储介质并生成文件系统 */
		ret = f_mkfs("1:", 0, buff, FF_MAX_SS);	
		if(ret == FR_OK){
			FATFS_LOG("%s|%s|%d W25Q64 格式化成功\r\n",__FILE__, __func__, __LINE__);
			
			/* 为了严谨性,将设备卸载并重新挂载验证本次操作有效 */
			f_unmount("1:");
			ret = f_mount(fs, "1:", 1);
			if(ret == FR_OK){
				FATFS_LOG("%s|%s|%d W25Q64 重新挂载成功\r\n",__FILE__, __func__, __LINE__);
			}else{
				FATFS_LOG("%s|%s|%d W25Q64 重新挂载失败 原因: ret = %d\r\n",__FILE__, __func__, __LINE__, ret);
			}
		}else{
			FATFS_LOG("%s|%s|%d W25Q64 格式化失败 原因: ret = %d\r\n",__FILE__, __func__, __LINE__, ret);
		}
	}else{
		FATFS_LOG("%s|%s|%d W25Q64 挂载失败 原因: ret = %d\r\n",__FILE__, __func__, __LINE__, ret);
	}
}

/**
 * @brief  打开或创建一个长文件名的文件
 * @param  fp   指向文件对象 (FIL) 的指针
 * @param  path 要打开/创建的文件路径 (如 "1:test.txt")
 * @note   如果文件存在就直接打开并清空源文件，如果文件不存在就使用以可读可写的方式创建文件。
 *         使用的是 FA_CREATE_ALWAYS | FA_WRITE | FA_READ 标志。
 * @retval 无
 */
void BSP_Fatfs_OpenFile(FIL *fp, const TCHAR* path)
{
	FRESULT ret;
	
	ret = f_open(fp, path, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if(ret != FR_OK){
		FATFS_LOG("%s|%s|%d 文件打开/创建失败 错误码: %d\r\n",__FILE__, __func__, __LINE__, ret);
	}
}

/**
 * @brief  向打开的文件中写入通用数据
 * @param  fp   指向已打开文件对象 (FIL) 的指针
 * @param  buff 指向要写入数据缓冲区的指针（使用 void* 适配任何数据类型）
 * @param  btw  要写入的字节数 (Bytes To Write)
 * @param  bw   指向用于存储实际已写入字节数变量的指针 (Bytes Written)
 * @note   所有的写入操作建立在文件句柄有效的前提上。
 * @retval 无
 */
void BSP_Fatfs_WriteData(FIL *fp, void* buff, UINT btw, UINT *bw)
{
	FRESULT ret;

	/* 首先判断文件句柄是否有效 */
	if(fp != NULL){
		ret = f_write(fp, buff, btw, bw);
		FATFS_LOG("%s|%s|%d 共写入%d个字节数据\r\n",__FILE__, __func__, __LINE__, *bw);
		if(ret != FR_OK){
			FATFS_LOG("%s|%s|%d 文件写入失败 错误码: %d\r\n",__FILE__, __func__, __LINE__, ret);
		}
	}
}

/**
 * @brief  从打开的文件中读取通用数据
 * @param  fp   指向已打开文件对象 (FIL) 的指针
 * @param  buff 指向用于存储读取出数据的缓冲区的指针（使用 void* 适配任何数据类型）
 * @param  btr  要读取的字节数 (Bytes To Read)
 * @param  br   指向用于存储实际已读取字节数变量的指针 (Bytes Read)
 * @note   在读取前首先要确认光标位置：若文件刚刚写入，光标位置会被置于末尾；
 *         如果此时不移动它的光标位置就去读取的话，就会读到空的数据。
 *         所以在读取前首先要判断它的光标位置，如果不为0，就将它的位置自动拨回文件开头。
 *         (和 C库 函数一样，f_tell() 能够检测光标位置偏移量)
 * @retval 无
 */
void BSP_Fatfs_ReadData(FIL *fp, void *buff, UINT btr, UINT *br)
{
	FRESULT ret;

	/* 首先判断文件句柄是否有效 */
	if(fp != NULL){
		/* 将文件光标设置为文件开头 */
		if(f_tell(fp) != 0){
			f_lseek(fp, 0);			
		}
		
		ret = f_read(fp, buff, btr, br);
		if(ret != FR_OK){
			FATFS_LOG("%s|%s|%d 文件读取失败 错误码: %d\r\n",__FILE__, __func__, __LINE__, ret);
		}
		printf("%s|%s|%d 共读取到%d个字节数据\r\n",__FILE__, __func__,__LINE__, *br);
	}
}

/**
 * @brief  关闭文件
 * @param  fp 指向已打开文件对象 (FIL) 的指针
 * @note   操作完毕后必须关闭文件，要不然数据不会被同步刷新并正确写入到物理文件中。
 * @retval 无
 */
void BSP_Fatfs_CloseFile(FIL *fp)
{
	FRESULT ret;

	/* 首先判断文件句柄是否有效 */
	if(fp != NULL){
		ret = f_close(fp);				
		if(ret != FR_OK){
			FATFS_LOG("%s|%s|%d 文件关闭失败 错误码: %d\r\n",__FILE__, __func__, __LINE__, ret);
		}
	}
}
