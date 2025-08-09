

#书写规则：将最后生成的可执行文件放在最上边，然后指定它所依赖的文件
bin/calc:obj/mycalc.o obj/math.o
	gcc -o bin/calc obj/math.o obj/mycalc.o
obj/math.o:src/math.c
	gcc -o obj/math.o -c src/math.c
obj/mycalc.o:src/mycalc.c
	gcc -o obj/mycalc.o -c src/mycalc.c -I include/
#这里的mycalc.c文件里用到include目录下的头文件，所以要指定头文件的目录
