CC:=gcc
SRC:=src/io.c
SRC+=src/cat.c
INCLUDE:=include/
TARGET:=bin/cat
LIBS:=-L lib/
STAT_LIB:=lib/libio.a
DEPEND:=obj/io.o
DEPEND+=obj/cat.o
CFLAGS:=-I $(INCLUDE)
LFLAGS:=-lio


$(TARGET):$(SRC)
	$(CC) -o $@ $(CFLAGS) $(SRC) $(LIBS) $(LFLAGS)
$(STAT_LIB):$(DEPEND)
	ar rcs $@ $<
$(DEPEND):$(SRC)
	$(CC) -o $@ $(CFLAGS) $<

debug:
	echo $(CC)
	echo $(SRC)
	echo $(INCLUDE)
	echo $(CFLAGS)
	echo $(LIBS)
	echo $(LFALGS)
	echo $(DEPEND)
	echo $(TARGET)

.PHONY:clean
clean:
	rm -rf $(TATGER) $(DEPEND)
