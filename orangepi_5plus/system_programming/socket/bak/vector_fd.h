#ifndef __VECTOR_H__
#define __VECTOR_H__

typedef struct{
	int *fd;
	int counter;
	int max_counter;
}vector_t;

extern vector_t *create_vector_fd(void);

extern void destroy_vector_fd(vector_t *);

extern int get_fd(vector_t *, int index);

extern void remove_fd(vector_t *, int fd);

extern void add_fd(vector_t *, int fd);

#endif
