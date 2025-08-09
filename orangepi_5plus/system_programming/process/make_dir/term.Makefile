CC:=gcc
CFLAGS:=-Iinclude
CFLAGS+=-c
TARGET:=bin/process_term
DEPEND:=obj/process_term.o
SRC:=src/process_term.c
CLEAN_CMD:=rm -rf

$(TARGET):$(DEPEND)
	$(CC) -o $@ $<
$(DEPEND):$(SRC)
	$(CC) -o $@ $(CFLAGS) $<


.PHONYL:clean
clean:
	$(CLEAN_CMD) $(TARGET) $(DEPEND)
