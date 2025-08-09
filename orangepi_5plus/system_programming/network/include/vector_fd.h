#ifndef __VECTOR__H__
#define __VECTOR__H__

typedef struct{
	int *fd;
	int counter;
	int max_counter;
}VectorFD;

extern VectorFD  *create_vector(void);
extern void 	 destroy_vector(VectorFD *vector);
extern void 	 add_vector(VectorFD *vector, int fd);
extern void 	 remove_vector(VectorFD *vector, int fd);
extern int 		 get_vector(VectorFD *vector, int index);

#endif
