#ifndef VECTOR_FD_H
#define VECTOR_FD_H

typedef struct{
    int *fd;
    int counter;
    int max_counter;
}vector_t;

extern vector_t *create_vector(void);
extern void destroy_vector(vector_t *vec);
extern void vector_add(vector_t *vec, int fd);
extern void vector_remove(vector_t *vec, int fd);
extern int vector_get(vector_t *vec, int index);

#endif