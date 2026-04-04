#include "bsp_nvic_group.h"

/**
 * @file    bsp_nvic_group.c
 * @brief   NVIC 优先级分组配置实现文件
 * @details
 * 本文件实现 NVIC 优先级分组初始化。
 */

/**
 * @brief 配置 NVIC 优先级分组
 * @details
 * 当前配置为 `NVIC_PriorityGroup_2`，表示：
 * - 2 位抢占优先级
 * - 2 位子优先级
 *
 * @note
 * 所有外设中断优先级设置都必须与该分组方式匹配，否则优先级行为可能与预期不一致。
 */
void BSP_NVIC_GroupConfig(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}