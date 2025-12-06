#include "vessel_fd.h"
#include <stdlib.h>
#include <assert.h>


/**
 * @brief 创建一个 fd 容器，初始容量为 5
 */
vessel_fd_t* create_vessel(void)
{
    vessel_fd_t *ves = (vessel_fd_t *)calloc(1, sizeof(vessel_fd_t));
    assert(ves != NULL);

    ves->fd = (int *)calloc(5, sizeof(int));
    assert(ves->fd != NULL);
    ves->counter = 0;
    ves->max_counter = 5;

    return ves;
}

/**
 * @brief 销毁容器并释放成员
 */
void destroy_vessel(vessel_fd_t *ves)
{
    assert(ves != NULL);

    free(ves->fd);
    free(ves);
}

/**
 * @brief 内部函数：扩容，容量变为原来的 2 倍
 */
static void vessel_expand(vessel_fd_t *ves)
{
    assert(ves != NULL);

    ves->max_counter *= 2;
    int *new_fd = (int *)realloc(ves->fd, 
            sizeof(int) * ves->max_counter);
    assert(new_fd != NULL);
    ves->fd = new_fd;
}

/**
 * @brief 添加 fd，如容量不够则扩容
 */
void add_vessel_fd(vessel_fd_t *ves, int fd)
{
    assert(ves != NULL);
    if(ves->counter == ves->max_counter){
        vessel_expand(ves);
    }
    ves->fd[ves->counter++] = fd;
}

/**
 * @brief 移除指定 fd
 * 删除方式：用最后一个覆盖要删除的位置，O(1)
 */
void remove_vessel_fd(vessel_fd_t *ves, int fd)
{
    assert(ves != NULL);

    int i;
    for(i = 0; i < ves->counter; i++){
        if(ves->fd[i] == fd){
            // 用最后一个元素覆盖，避免整体左移
            ves->fd[i] = ves->fd[ves->counter - 1];
            ves->counter--;
            return;
        }
    }
}

/**
 * @brief 获取指定索引的 fd，下标非法返回 -1
 */
int get_vessel_fd(vessel_fd_t *ves, int index)
{
    assert(ves != NULL);

    if(index < 0 || index > ves->counter - 1)
        return -1;
    
    return ves->fd[index];
}

/**
 * @brief 查找指定 fd 在容器中的下标
 * 找不到返回 -1
 */
int find_vessel_fd(vessel_fd_t *ves, int fd)
{
    assert(ves != NULL);

    int i;
    for(i = 0; i < ves->counter; i++){
        if(ves->fd[i] == fd){
            return i;
        }
    }
    return -1;
}