#ifndef _IO_H
#define _IO_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

void copy_function(int fdin, int fdout);
void set_fl(int fd, int flag);
void clr_flag(int fd, int flag);

#endif 
