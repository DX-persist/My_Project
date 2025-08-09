CC:=gcc
SRC:=src/process_fork5.c
DEPEND:=obj/process_fork5.o
TARGET:=bin/process_fork5
CFLAGS:=-I include/
CFLAGS+=-c
CLEAN_CMD:=rm -rf

$(TARGET):$(DEPEND)
	$(CC) -o $@ $<

$(DEPEND):$(SRC)
	$(CC) -o $@ $(CFLAGS) $<

.PHONY:clean
clean:
	$(CLEAN_CMD) $(TARGET) $(DEPEND)
