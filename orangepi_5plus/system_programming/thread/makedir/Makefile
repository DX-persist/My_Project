#编译器和编译选项
CC:=gcc
CFLAGS:=-I include/
LFLAGS:=-lpthread

#源文件和目标文件
SRC:=src/pthread_race1.c
OBJ:=obj/pthread_race1.o
TARGET:=bin/pthread_race1

#链接阶段
$(TARGET):$(OBJ)
	$(CC) -o $@ $< $(LFLAGS)

#编译阶段
$(OBJ):$(SRC)
	$(CC) -o $@ $(CFLAGS) -c $<

#清理
.PHONY:clean
clean:
	rm -rf $(OBJ) $(TARGET)
