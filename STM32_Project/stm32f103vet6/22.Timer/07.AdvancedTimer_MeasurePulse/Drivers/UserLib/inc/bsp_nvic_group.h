#ifndef BSP_NVIC_GROUP_H
#define BSP_NVIC_GROUP_H

#include "stm32f10x.h"

/**
 * @file    bsp_nvic_group.h
 * @brief   NVIC 优先级分组配置接口头文件
 * @details
 * 本文件用于声明 NVIC 优先级分组初始化接口。
 */

/**
 * @brief 配置 NVIC 优先级分组
 * @details
 * 设置系统中断优先级分组方式，为后续外设中断优先级配置提供基础。
 *
 * @return 无返回值
 */
extern void BSP_NVIC_GroupConfig(void);

#endif