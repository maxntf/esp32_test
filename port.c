#include "port.h"

static PortHandler_t pHandler = {.IsOpen = FALSE};

/** OpenSerialPort
* @brief			Open serial port
* @param[in]				device: serial port name;
* @param[in]				baud_rate: baud rate;
* @return			Integer	0 or 1
*/
int OpenSerialPort(const char* device, uint32_t baud_rate)
{
	pHandler.port = CreateFileA(device, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (pHandler.port == INVALID_HANDLE_VALUE)
	{
		pHandler.IsOpen = FALSE;
		return 0;
	}
	BOOL success;
	// Configure read and write operations to time out after 100 ms.
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 0;
	timeouts.ReadTotalTimeoutConstant = SERIAL_RX_TO;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = SERIAL_TX_TO;
	timeouts.WriteTotalTimeoutMultiplier = 0;

	success = SetCommTimeouts(pHandler.port, &timeouts);
	if (!success)
	{
		CloseHandle(pHandler.port);
		pHandler.IsOpen = FALSE;
		return 0;
	}

	// Set the baud rate and other options.
	DCB state = { 0 };
	state.DCBlength = sizeof(DCB);
	state.BaudRate = baud_rate;
	state.ByteSize = 8;
	state.Parity = NOPARITY;
	state.StopBits = ONESTOPBIT;
	success = SetCommState(pHandler.port, &state);

	pHandler.IsOpen = TRUE;
	return 1;
}
/** CloseSerialPort
* @brief			Close serial port
* @return			
*/
void CloseSerialPort()
{
	if (pHandler.IsOpen) {
		pHandler.IsOpen = FALSE;
		CloseHandle(pHandler.port);
	}
}
/** WritePort
* @brief			Write serial port
* @param[in]				buffer: date buffer;
* @param[in]				size: data size;
* @return			-1 is error, 0 is no transmit data or 1
*/
int WritePort(uint8_t* buffer, size_t size)
{
	if (!pHandler.IsOpen) return -1;
	DWORD written;
	BOOL success = WriteFile(pHandler.port, buffer, size, &written, NULL);
	if (!success)
	{
		return -1;
	}
	if (written != size)
	{
		return 0;
	}
	return 1;
}
/** ReadPort
* @brief			Read serial port
* @param[out]				buffer: input buffer;
* @param[in]				size: buffer size;
* @return			-1 is error or received data length
*/
int ReadPort(uint8_t* buffer, size_t size)
{
	if (!pHandler.IsOpen) return -1;
	DWORD NoBytesRead;
	uint8_t b;
	uint32_t len = 0;
	do
	{
		if (ReadFile(pHandler.port, &b, 1, &NoBytesRead, NULL) == FALSE) break;
		if (NoBytesRead > 0) buffer[len++] = b;
	} while ((len <= size) && (NoBytesRead > 0));
	return len;
}
/** ReadDataWait
* @brief			Reading data with a timeout
* @param[out]				buffer: input buffer;
* @param[in]				size: buffer size;
* @param[in]				to: timeout;
* @return			0 or received data length
*/
uint32_t ReadDataWait(char* buffer, size_t size, uint32_t to)
{
	int res = 0;
	uint32_t offset = 0;
	to /= SERIAL_RX_TO;

	while ((offset < size) && (to > 0)) {
		res = ReadPort(&buffer[offset], size - offset);
		if (res < 0) return 0;
		if (res) {
			offset += res;
			to = 1;
		}
		else {
			to--;
		}
	}
	if (offset >= size) return 0;
	buffer[offset] = '\0';
	return offset;
}

