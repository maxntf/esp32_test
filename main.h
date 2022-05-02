#ifndef __MAIN_H
#define __MAIN_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Buffers size */
#define HOST_MAXLEN		100U		
#define PATH_MAXLEN		100U		
#define HEAD_SIZE		74U		
#define POST_BUF_SIZE	(unsigned int)(TX_PYLO_MAXLEN + HOST_MAXLEN + PATH_MAXLEN + HEAD_SIZE)
#define REQUEST_MAXLEN	POST_BUF_SIZE
#define RESPONSE_MAXLEN	RX_PYLO_MAXLEN
/* AtHandler selector masks */
#define CHECK_MSK(f,msk)	((f & msk) == msk)
#define MSK_OK				(uint8_t)0x1
#define MSK_SEND_OK			(uint8_t)0x1
#define MSK_INDICATOR		(uint8_t)0x1
#define MSK_ERROR			(uint8_t)0x2
#define MSK_WFCONN			(uint8_t)0x4
#define MSK_WFGIP			(uint8_t)0x8
#define WIFI_CONNECTED_MSK	(uint8_t)(MSK_OK | MSK_WFCONN | MSK_WFGIP)
#define MSK_IPCONNECT		(uint8_t)0x4	
#define IP_CONNECTED_MSK	(uint8_t)(MSK_OK | MSK_IPCONNECT)
/* Request type */
typedef enum { GET = 0, POST = !GET } Method_t;
/* Get terminal line tokens */
int GetTokens(char* str, char** tokens, size_t size);
/* Compare tokens with masks */
uint8_t CmpTokens(const char** mask_str, size_t mcnt, char** tokens_str, size_t tcnt);
/* Command input string waiter */
void ConsoleInputWait(char* buf, size_t size);
/* Command input handler */
int InputHandler(char* mem, size_t size, char* method, char* url, char* payload);
/* Parse Url */
int ParseUrl(char* url_str, char** host, char** path);
/* AT command handler */
void AtHandler(char const* mask[], size_t size, uint8_t* fmask, char* cmd_str, uint32_t to);
/* ESP32 initialization */
int Esp32Init();
/* WiFi connection */
int ConnectWiFi(char* ssid, char* pwd);
/* Send GET or POST request */
int SendRequest(Method_t method, char* url_str, char* payload);

#endif /* __MAIN_H */
