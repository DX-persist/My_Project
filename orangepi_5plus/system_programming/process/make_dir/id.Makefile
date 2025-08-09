CC:=gcc
TARGET:=bin/process_id
DEPEND:=obj/process_id.o
SRC:=src/process_id.c
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
