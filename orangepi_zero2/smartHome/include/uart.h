#ifndef __UART_H
#define __UART_H

#define SERIAL_DEV "/dev/ttyS5"
#define BAUD 115200
//#define RXD 4
//#define TXD 3
int serialOpen (const char *device, const int baud);
void serialClose (const int fd);
int serialSendstring (const int fd, const unsigned char *s, int len);
int serialGetstring (const int fd, unsigned char *buffer);


#endif