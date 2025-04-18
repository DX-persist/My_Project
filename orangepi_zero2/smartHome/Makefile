CC := aarch64-linux-gnu-gcc		#指定编译工具
SRC := $(shell find src -name *.c)		#//在当前目录下的src目录找所有的.c文件
INCLUDE := ./include	\
		./3rd/usr/local/include \
		./3rd/usr/include \
		./3rd/usr/include/aarch64-linux-gnu \
		./3rd/usr/include/aarch64-linux-gnu/python3.10 \
		./3rd/usr/include/python3.10 \
		./3rd/usr/include/python3.10/cpython \
		./3rd/usr/include/python3.10/internal \

LIBS_PATH := ./3rd/usr/local/lib	\
			./3rd/lib/aarch64-linux-gnu \
			./3rd/usr/lib/aarch64-linux-gnu \
			./3rd/usr/lib/aarch64-linux-gnu/cmake \
			./3rd/usr/lib/aarch64-linux-gnu/pkgconfig \
			./3rd/usr/lib/python3.10

OBJ := $(subst src/,obj/,$(SRC:.c=.o))

TARGET := obj/smarthome

CFLAGS := $(foreach item, $(INCLUDE), -I$(item)) 	#-I./include
LDFLAGS := $(foreach item, $(LIBS_PATH), -L$(item))	#-L ./3rd/usrlocal/lib

LIBS := -lwiringPi -lwiringPiDev -lm -lcrypt -lrt -lpython3.10 -lpthread -lexpat -lz 
$(shell mkdir -p obj)

obj/%.o:src/%.c		#obj/.o文件依赖于src/的.c文件
	$(CC) -o $@ -c $< $(CFLAGS)

$(TARGET) :$(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)
	scp obj/smarthome src/face.py ini/gdevice.ini orangepi@192.168.0.6:/home/orangepi
#	scp obj/smarthome src/face.py ini/gdevice.ini orangepi@192.168.43.7:/home/orangepi
complie :$(TARGET)
clean: 
	rm -f $(TARGET) $(OBJ)

debug: 
	@echo $(CC)
	@echo $(SRC)
	@echo $(INCLUDE)
	@echo $(OBJ)
	@echo $(TARGET)
	@echo $(CFLAGS)
	@echo $(LDFLAGS)
	@echo $(LIBS)

.PHONY: clean complie debug
