#ifndef BSP_FATFS_H
#define BSP_FATFS_H

#ifdef __cplusplus
extern "C"{
#endif

#include "ff.h"

/**
 * @brief 调试日志宏，开启 FATFS_DEBUG_ON 后输出调试信息
 */
#define FATFS_DEBUG_ON
#ifdef FATFS_DEBUG_ON
	#include <stdio.h>
	#define FATFS_LOG(format, ...)	printf(format, ##__VA_ARGS__)
#else
	#define FATFS_LOG(format, ...)
#endif

extern void BSP_FatFs_Init(FATFS *fs, BYTE *buff);
extern void BSP_Fatfs_OpenFile(FIL *fp, const TCHAR* path);
extern void BSP_Fatfs_WriteData(FIL *fp, void* buff, UINT btw, UINT *bw);
extern void BSP_Fatfs_ReadData(FIL *fp, void *buff, UINT btr, UINT *br);
extern void BSP_Fatfs_CloseFile(FIL *fp);

#ifdef __cplusplus
}
#endif

#endif /* BSP_FATFS_H */
