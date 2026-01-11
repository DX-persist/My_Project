#include "vessel_fd.h"
#include <stdlib.h>
#include <assert.h>

vessel_fd_t *create_vessel()
{
	vessel_fd_t *ves = (vessel_fd_t *)calloc(1, sizeof(vessel_fd_t));
	if(ves == NULL){
		return NULL;
	}
	ves->fd = (int *)calloc(5, sizeof(int));
	if(ves->fd == NULL){
		free(ves);
		return NULL;
	}
	ves->counter = 0;
	ves->max_counter = 5;

	return ves;
}

void destroy_vessel(vessel_fd_t *ves)
{
	assert(ves != NULL);
	assert(ves->fd != NULL);

	free(ves->fd);
	free(ves);
}

static void vessel_expand(vessel_fd_t *ves)
{
	assert(ves != NULL);
	assert(ves->fd != NULL);

	int new_size = ves->max_counter * 2;
	int *new_fd = (int *)realloc(ves->fd, new_size * sizeof(int));
	if(new_fd == NULL){
		free(ves->fd);
		return;
	}
	ves->fd = new_fd;
	ves->max_counter = new_size;
}

void add_vessel_fd(vessel_fd_t *ves, int fd)
{
	assert(ves != NULL);
	assert(ves->fd != NULL);

	if(ves->counter == ves->max_counter){
		vessel_expand(ves);
	}
	ves->fd[ves->counter++] = fd; 
}

void remove_vessel_fd(vessel_fd_t *ves, int fd)
{
	assert(ves != NULL);
	assert(ves->fd != NULL);

	int i = 0;
	for(; i < ves->counter; i++){
		if(ves->fd[i] == fd){
			ves->fd[i] = ves->fd[ves->counter - 1];
			ves->counter--;
			return;
		}
	}
}

int get_vessel_fd(vessel_fd_t *ves, int index)
{
	assert(ves != NULL);
	assert(ves->fd != NULL);

	if(index < 0 || index > ves->counter - 1)	return -1;

	return ves->fd[index];
}
