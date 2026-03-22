/**
 * @file diskio.c
 * @brief FatFs 底层磁盘 I/O 模块
 * @details 若已有可用的存储控制模块，应通过胶合函数将其挂接到 FatFs，
 *          而非直接修改 FatFs 源码。本文件是将各种现有存储控制模块以
 *          统一 API 挂接到 FatFs 的胶合函数示例。
 * @copyright (C)ChaN, 2025
 */

#include "ff.h"     /**< FatFs 基本定义 */
#include "diskio.h" /**< FatFs 磁盘 I/O 接口声明 */

#include "bsp_w25q64.h"

/**
 * @defgroup PhysDriveNum 物理驱动器编号映射
 * @brief 各驱动器的物理驱动器编号映射关系
 * @{
 */
#define DEV_SD_CRAD 	0	/**< 将 SD 卡映射到物理驱动器 0 */
#define DEV_SPI_FLASH	1	/**< 将 W25Q64 Flash 闪存映射到物理驱动器 1 */
/** @} */


/**
 * @brief 获取磁盘驱动器的当前状态。
 * @param pdrv 用于标识驱动器的物理驱动器编号。
 * @return 磁盘状态 @ref DSTATUS。若驱动器编号无效则返回 @ref STA_NOINIT。
 */
DSTATUS disk_status(BYTE pdrv)
{
	switch(pdrv){
#if 0
		case DEV_SD_CRAD:
			BSP_SD_ReadID();			
			return 0;		
#endif
		case DEV_SPI_FLASH:
			/* 调用读取 W25Q64 的JEDEC ID 函数，将其返回值与之比较，就可以确定 W25Q64 的状态了 */
			if(BSP_W25Q64_ReadID() == W25Q64_JEDEC_ID){
				return 0;
			}else{
				return STA_NOINIT;			/**< 如果检测到两个 ID 值不相同则说明不是 W25Q64 设备 */
			}
		default:
			return STA_NOINIT;			/**< 如果检测到两个 ID 值不相同则说明不是 W25Q64 设备 */
	}
}


/**
 * @brief 初始化磁盘驱动器。
 * @param pdrv 用于标识驱动器的物理驱动器编号。
 * @return 磁盘状态 @ref DSTATUS。若驱动器编号无效则返回 @ref STA_NOINIT。
 */
DSTATUS disk_initialize(BYTE pdrv)
{
	switch(pdrv){
#if 0
		case DEV_SD_CRAD:
			BSP_SD_Init();			/**< 调用 SD 卡初始化函数 */
			return 0;		
#endif
		case DEV_SPI_FLASH:
			BSP_W25Q64_Init();		/**< 调用 W25Q64 初始化函数 */
			return disk_status(DEV_SPI_FLASH);	/**< 初始化后立即检测设备状态 */

		default:
			return STA_NOINIT;			/**< 其他不认识的物理设备返回未初始化 */
	}
}

/**
 * @brief 从磁盘驱动器读取扇区。
 * @param pdrv 用于标识驱动器的物理驱动器编号。
 * @param buff 指向用于存储读取数据的缓冲区的指针。
 * @param sector 起始扇区的 LBA 地址。
 * @param count 要读取的扇区数量。
 * @return 操作结果 @ref DRESULT。若驱动器编号无效则返回 @ref RES_PARERR。
 */
DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
	switch(pdrv){
#if 0
		case DEV_SD_CRAD:
			BSP_SD_ReadBuff();
			return RES_OK;		
#endif
		case DEV_SPI_FLASH:

			/* 由于这里的 sector 是扇区号而非是字节，所以这里要转换为字节
			 * 对于W25Q64来说一个扇区(4k) = 4096Bytes，所以这里要
			 * 写入的字节数为扇区号 * 一个扇区有多少字节
			 * 写入地址为扇区号 * 一个扇区有多少字节
			 *
			 * 这里从文件i的后6M字节读取,留空前2M字节用来存放字模等数据
			 * 2M = 2,097,152 Bytes = 512 sectors
			 * */
			sector += 512;
			BSP_W25Q64_BufferRead(buff, count * W25Q64_SECTOR_SIZE, sector * W25Q64_SECTOR_SIZE);	
			return RES_OK;
		default:
			return RES_PARERR;			/**< 其他不认识的物理设备返回参数错误 */
	}
}


#if FF_FS_READONLY == 0
/**
 * @brief 向磁盘驱动器写入扇区。
 * @param pdrv 用于标识驱动器的物理驱动器编号。
 * @param buff 指向待写入数据的指针。
 * @param sector 起始扇区的 LBA 地址。
 * @param count 要写入的扇区数量。
 * @return 操作结果 @ref DRESULT。若驱动器编号无效则返回 @ref RES_PARERR。
 */
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
	DWORD write_addr;
	switch(pdrv){
#if 0
		case DEV_SD_CRAD:
			BSP_SD_WriteBuff();
			return RES_OK;		
#endif
		case DEV_SPI_FLASH:
			/* 这里有一点需要注意,假如此时要写入3个扇区的数据,假设起始扇区为10
			 * 第一次写入地址为 10 * 4096, 写入数据的大小为 4096 个字节的数据, 即下标为 0 ~ 4095 的数据
			 * 第二次写入地址为 11 * 4096, 写入数据的大小为 4096 个字节的数据, 即下标为 4096 ~ 8191 的数据
			 * 第三次写入地址为 12 * 4096, 写入数据的大小为 4096 个字节的数据, 即下标为 8192 ~ 12287 的数据
			 *
			 * 若这里直接调用BSP_W25Q64_BufferWrite(buff, count * 4096, sector * 4096);直接就是错的, 因为
			 * 擦除和写入的地址一直是 sector * 4096, 写入的数据一直是 count * 4096, 如果换成上边的例子就是
			 * 一直在擦除和写入同一片地址，并且写进去的数据也只有一部分数据。所以这里应该确定 count 的大小
			 * 若 count > 1, 则说明要写入的地址和数据都要进行偏移
			 * */
			/* 此处同理 */
			sector += 512;
			for(UINT i = 0; i < count; i++){
				/* 利用扇区号求出写入/擦除地址 */
				write_addr = (sector + i) * W25Q64_SECTOR_SIZE;
				/* Flash 特性：先擦除后写入 */
				BSP_W25Q64_Sector_Erase(write_addr);
				/* 当 i = 1 时，buff + 4096 即在(sector + i) * 4096 的位置
				 * 存放buff[4096] - buff[8191]的数据,此时刚好跨过了已经写入
				 * 的 4kb 数据 */
				BSP_W25Q64_BufferWrite((uint8_t *)(buff + i * W25Q64_SECTOR_SIZE), W25Q64_SECTOR_SIZE,  write_addr);
			}
			return RES_OK;
		default:
			return RES_PARERR;			/**< 其他不认识的物理设备返回参数错误 */
	}
}
#endif


/**
 * @brief 执行杂项驱动器控制操作。
 * @param pdrv 物理驱动器编号（0..）。
 * @param cmd 控制命令码，可用命令参见 @ref DiskIoctlCmd。
 * @param buff 指向用于发送/接收控制数据的缓冲区的指针。
 * @return 操作结果 @ref DRESULT。若驱动器编号无效则返回 @ref RES_PARERR。
 */
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
	switch(pdrv){
#if 0
		case DEV_SD_CRAD:
			BSP_SD_WriteBuff();
			return RES_OK;		
#endif
		case DEV_SPI_FLASH:
			switch(cmd){
				case GET_SECTOR_COUNT:		/**< 获取扇区总数(由于这里从后2M字节写入,所以剩下的扇区数为 2048-512 sectors) */
					*(DWORD *)buff = W25Q64_SECTOR_COUNT - 512;
					return RES_OK;
				case GET_SECTOR_SIZE:		/**< 获取扇区大小 */
					*(WORD *)buff = W25Q64_SECTOR_SIZE;
					return RES_OK;
				case GET_BLOCK_SIZE:		/**< 获取擦除块大小 */
					*(DWORD *)buff = W25Q64_ERASE_BLOCK_SIZE;
					return RES_OK;
				case CTRL_SYNC:			/**< 完成所有未完成的写入过程 */
					return RES_OK;
			}
	}
    return RES_PARERR;
}

/* 返回一个固定的默认时间: 2026年1月1日 00:00:00 */
DWORD get_fattime (void)
{
    return  ((DWORD)(2026 - 1980) << 25) /* 年份，从 1980 算起 */
            | ((DWORD)2 << 21)           /* 月份 */
            | ((DWORD)22 << 16)           /* 日期 */
            | ((DWORD)0 << 11)           /* 小时 */
            | ((DWORD)0 << 5)            /* 分钟 */
            | ((DWORD)0 >> 1);           /* 秒除以2 */
}
