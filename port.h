#include <stdint.h>
#include <windows.h>
#include "config.h"

typedef struct {
	HANDLE port;
	BOOL IsOpen;
}PortHandler_t;

int OpenSerialPort(const char* device, uint32_t baud_rate);
void CloseSerialPort();
int WritePort(uint8_t* buffer, size_t size);
uint32_t ReadPort(uint8_t* buffer, size_t size);
int ReadLine(uint8_t* buffer, size_t size, uint32_t to);
uint32_t ReadDataWait(char* buf, size_t size, uint32_t to);
