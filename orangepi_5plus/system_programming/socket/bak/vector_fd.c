#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "vector_fd.h"

vector_t *create_vector_fd(void)
{
	vector_t *vtr = (vector_t *)calloc(1, sizeof(vector_t));
	assert(vtr != NULL);
	
	vtr->fd = (int *)calloc(5, sizeof(int));
	assert(vtr->fd != NULL);
	vtr->counter = 0;
	vtr->max_counter = 0;

	return vtr;
}

void destroy_vector_fd(vector_t * vtr)
{
	assert(vtr != NULL);

	free(vtr->fd);
	free(vtr);
}

int get_fd(vector_t *vtr, int index)
{
	assert(vtr != NULL);

	if(index < 0 || index > vtr->counter-1)
		return 0;
	return vtr->fd[index];
}

static int indexof(vector_t *vtr, int fd)
{
	int i = 0;
	for(; i < vtr->counter; i++){
		if(vtr->fd[i] == fd)
			return i;
	}
	return -1;
}

void remove_fd(vector_t *vtr, int fd)
{
	assert(vtr != NULL);
	
	int index = indexof(vtr, fd);
	if(index == -1)	return;
	
	for(int i = index; i < vtr->counter-1; i++){
		vtr->fd[i] = vtr->fd[i+1];
	}
	vtr->counter--;
}
static void extension(vector_t *vtr)
{
	assert(vtr != NULL);
	
	if(vtr->counter >= vtr->max_counter){
		vector_t *new_vtr = (vector_t *)realloc(vtr, sizeof(vector_t) * (vtr->counter + 5));
		if(new_vtr == NULL){
			perror("realloc failed");
			free(vtr);
		}
		free(vtr);
		vtr = new_vtr;
		vtr->max_counter += 5;
	}
}

void add_fd(vector_t *vtr, int fd)
{
	assert(vtr != NULL);

	extension(vtr);
	vtr->fd[vtr->counter++] = fd;
}

