#include "bsp_nvic_group.h"

void BSP_NVIC_GroupConfig(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}
