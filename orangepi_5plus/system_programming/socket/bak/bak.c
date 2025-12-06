#include <stdlib.h>
#include <assert.h>
#include "bak.h"

vector_t *create_vector(void)
{
    vector_t *vec = (vector_t *)calloc(1, sizeof(vector_t));
    assert(vec != NULL);

    vec->fd = (int *)calloc(5, sizeof(int));
    assert(vec->fd != NULL);
    vec->counter = 0;
    vec->max_counter = 5;

    return vec;
}

void destroy_vector(vector_t *vec)
{
    assert(vec != NULL);

    free(vec->fd);
    free(vec);
}

static void vector_expand(vector_t *vec)
{
    assert(vec != NULL);

    //将整个动态数组扩大到原来的两倍
    vec->max_counter *= 2;
    int *new_fd = (int *)realloc(vec->fd, vec->max_counter * sizeof(int));
    assert(new_fd != NULL);
    //释放之前的空间并把指向重新分配后的空间
    vec->fd = new_fd;
}

void vector_add(vector_t *vec, int fd)
{
    if(vec->counter == vec->max_counter){
        vector_expand(vec);
    }
    vec->fd[vec->counter++] = fd;
}

void vector_remove(vector_t *vec, int fd)
{
    assert(vec != NULL);

    int i;
    for(i = 0; i < vec->counter; i++){
        if(vec->fd[i] == fd){
            //最后一个覆盖当前
            vec->fd[i] = vec->fd[vec->counter-1];
            vec->counter--;
            return;
        }
    }
}

int vector_get(vector_t *vec, int index)
{
    assert(vec != NULL);

    if(index < 0 || index > vec->counter)
        return 0;

    return vec->fd[index];
}