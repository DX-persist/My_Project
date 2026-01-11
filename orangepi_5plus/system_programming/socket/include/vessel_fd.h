#ifndef VESSEL_FD_H
#define VESSEL_FD_H

typedef struct{
	int *fd;
	int counter;
	int max_counter;
}vessel_fd_t;

extern vessel_fd_t *create_vessel(void);

extern void destroy_vessel(vessel_fd_t *ves);

extern void add_vessel_fd(vessel_fd_t *ves, int fd);

extern void remove_vessel_fd(vessel_fd_t *ves, int fd);

extern int get_vessel_fd(vessel_fd_t *ves, int index);

#endif
