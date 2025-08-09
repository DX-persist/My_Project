#include "socket.h"
#include "vector_fd.h"

static void extension(VectorFD *vector)
{
	assert(vector != NULL);
	if(vector->counter >= vector->max_counter){
		//使用realloc函数重新分配空间后原有的数据会直接拷贝到新分配的内存中,不再需要使用memcpy函数
		int *temp = (int *)realloc(vector->fd, 
				sizeof(int) * (vector->counter + 5));
		assert(temp != NULL);
		free(vector->fd);
		vector->fd = temp;
		vector->max_counter += 5;
	}
}

static int indexof(VectorFD *vector, int fd)
{
	assert(vector != NULL);

	int i = 0;
	for(; i < vector->counter; i++){
		if(vector->fd[i] == fd)
			return i;
	}
	return -1;
}

VectorFD *create_vector(void)
{
	VectorFD *vector = (VectorFD *)calloc(1, sizeof(VectorFD));
	assert(vector != NULL);

	vector->counter = 0;
	vector->max_counter = 0;
	vector->fd = (int *)calloc(5, sizeof(int));
	assert(vector->fd != NULL);

	return vector;
}

void destroy_vector(VectorFD *vector)
{
	assert(vector != NULL);

	free(vector->fd);		//先释放vector里边成员所占有的堆空间再释放vector
	free(vector);
}

void add_vector(VectorFD *vector, int fd)
{
	assert(vector != NULL);

	extension(vector);
	vector->fd[vector->counter++] = fd;
	//上边这句等价于：
	//vector->fd[vector->counter] = fd;
	//vector->counter++;
}

void remove_vector(VectorFD *vector, int fd)
{
	assert(vector != NULL);
	
	int index = indexof(vector, fd);
	if(index == -1) return;
	int i = index;

	for(; i < vector->counter-1; i++){
		vector->fd[i] = vector->fd[i+1];
	}
	vector->counter--;
}

int get_vector(VectorFD *vector, int index)
{
	assert(vector != NULL);

	if(index < 0 || index > vector->counter-1){
		return 0;
	}
	return vector->fd[index];
}
