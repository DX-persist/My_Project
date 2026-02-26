/**
 * @file diskio.h
 * @brief 底层磁盘接口模块头文件
 * @copyright (C)ChaN, 2025
 */

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 磁盘函数的状态类型 */
typedef BYTE DSTATUS;

/**
 * @brief 磁盘函数的返回结果
 */
typedef enum {
    RES_OK = 0, /**< 0: 操作成功 */
    RES_ERROR,  /**< 1: 读写错误 */
    RES_WRPRT,  /**< 2: 写保护 */
    RES_NOTRDY, /**< 3: 设备未就绪 */
    RES_PARERR  /**< 4: 无效参数 */
} DRESULT;

/**
 * @defgroup DiskStatus 磁盘状态位 (DSTATUS)
 * @{
 */
#define STA_NOINIT  0x01 /**< 驱动器未初始化 */
#define STA_NODISK  0x02 /**< 驱动器中没有介质 */
#define STA_PROTECT 0x04 /**< 写保护 */
/** @} */

/**
 * @defgroup DiskIoctlCmd disk_ioctl 函数控制命令码
 * @{
 */

/** @defgroup GenericCmdFatFs 通用命令（由 FatFs 使用）*/
/** @{ */
#define CTRL_SYNC        0 /**< 完成待写入的数据（FF_FS_READONLY == 0 时需要） */
#define GET_SECTOR_COUNT 1 /**< 获取介质容量（FF_USE_MKFS == 1 时需要） */
#define GET_SECTOR_SIZE  2 /**< 获取扇区大小（FF_MAX_SS != FF_MIN_SS 时需要） */
#define GET_BLOCK_SIZE   3 /**< 获取擦除块大小（FF_USE_MKFS == 1 时需要） */
#define CTRL_TRIM        4 /**< 通知设备指定扇区块上的数据已不再使用（FF_USE_TRIM == 1 时需要） */
/** @} */

/** @defgroup GenericCmdNonFatFs 通用命令（不由 FatFs 使用）*/
/** @{ */
#define CTRL_POWER  5 /**< 获取/设置电源状态 */
#define CTRL_LOCK   6 /**< 锁定/解锁介质弹出 */
#define CTRL_EJECT  7 /**< 弹出介质 */
#define CTRL_FORMAT 8 /**< 在介质上创建物理格式 */
/** @} */

/** @defgroup MMCCmd MMC/SDC 专用 ioctl 命令（不由 FatFs 使用）*/
/** @{ */
#define MMC_GET_TYPE   10 /**< 获取卡类型 */
#define MMC_GET_CSD    11 /**< 获取 CSD */
#define MMC_GET_CID    12 /**< 获取 CID */
#define MMC_GET_OCR    13 /**< 获取 OCR */
#define MMC_GET_SDSTAT 14 /**< 获取 SD 状态 */
#define ISDIO_READ     55 /**< 从 SD iSDIO 寄存器读取数据 */
#define ISDIO_WRITE    56 /**< 向 SD iSDIO 寄存器写入数据 */
#define ISDIO_MRITE    57 /**< 向 SD iSDIO 寄存器掩码写入数据 */
/** @} */

/** @defgroup ATACmd ATA/CF 专用 ioctl 命令（不由 FatFs 使用）*/
/** @{ */
#define ATA_GET_REV   20 /**< 获取固件版本 */
#define ATA_GET_MODEL 21 /**< 获取型号名称 */
#define ATA_GET_SN    22 /**< 获取序列号 */
/** @} */

/** @} */ /* DiskIoctlCmd 结束 */

/**
 * @brief 初始化磁盘驱动器。
 * @param pdrv 用于标识驱动器的物理驱动器编号。
 * @return 磁盘状态 @ref DSTATUS。若驱动器编号无效则返回 @ref STA_NOINIT。
 */
DSTATUS disk_initialize(BYTE pdrv);

/**
 * @brief 获取磁盘驱动器的当前状态。
 * @param pdrv 用于标识驱动器的物理驱动器编号。
 * @return 磁盘状态 @ref DSTATUS。若驱动器编号无效则返回 @ref STA_NOINIT。
 */
DSTATUS disk_status(BYTE pdrv);

/**
 * @brief 从磁盘驱动器读取扇区。
 * @param pdrv 用于标识驱动器的物理驱动器编号。
 * @param buff 指向用于存储读取数据的缓冲区的指针。
 * @param sector 起始扇区的 LBA 地址。
 * @param count 要读取的扇区数量。
 * @return 操作结果 @ref DRESULT。若驱动器编号无效则返回 @ref RES_PARERR。
 */
DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count);

/**
 * @brief 向磁盘驱动器写入扇区。
 * @param pdrv 用于标识驱动器的物理驱动器编号。
 * @param buff 指向待写入数据的指针。
 * @param sector 起始扇区的 LBA 地址。
 * @param count 要写入的扇区数量。
 * @return 操作结果 @ref DRESULT。若驱动器编号无效则返回 @ref RES_PARERR。
 */
DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count);

/**
 * @brief 执行杂项驱动器控制操作。
 * @param pdrv 物理驱动器编号（0..）。
 * @param cmd 控制命令码，可用命令参见 @ref DiskIoctlCmd。
 * @param buff 指向用于发送/接收控制数据的缓冲区的指针。
 * @return 操作结果 @ref DRESULT。若驱动器编号无效则返回 @ref RES_PARERR。
 */
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);

#ifdef __cplusplus
}
#endif

#endif /* _DISKIO_DEFINED */