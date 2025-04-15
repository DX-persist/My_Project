#!/bin/bash

mkdir -p obj

#将onenet_mqtt.c和cJSON_Parse.c编译成.o文件
gcc -c src/onenet_mqtt.c -I include/ -o obj/onenet_mqtt.o
gcc -c src/cJSON_Parse.c -I include/ -o obj/cJSON_Parse.o

#将目标文件和main.c进行编译链接成一个可执行文件
gcc src/main.c obj/* -I include/ -o ./bin/onenet_mqtt -lmosquitto  -lcjson
