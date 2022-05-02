#ifndef __PORT_H
#define __PORT_H
#include <stdint.h>
#include <windows.h>
#include "config.h"

typedef struct {
	HANDLE port;
	BOOL IsOpen;
}PortHandler_t;

/* Open serial port */
int OpenSerialPort(const char* device, uint32_t baud_rate);
/* Close serial port */
void CloseSerialPort();
/* Write serial port */
int WritePort(uint8_t* buffer, size_t size);
/* Read serial port */
uint32_t ReadPort(uint8_t* buffer, size_t size);
/* Read serial port */
int ReadLine(uint8_t* buffer, size_t size, uint32_t to);
/* Reading data with a timeout */
uint32_t ReadDataWait(char* buf, size_t size, uint32_t to);

#endif /* __PORT_H */
