#include "main.h"
#include "port.h"
#include "utils.h"

/* WiFi setings */
const char* _ssid = WIFI_SSID;
const char* _pwd = WIFI_PWD;
/* Request and response buffers */
char reqBuf[REQUEST_MAXLEN];
char respBuf[RESPONSE_MAXLEN];
/* At commands response masks */
const char const* defMasks[] = { "OK", "ERROR" };
const char const* wifiMasks[] = { "OK", "ERROR", "WIFI CONNECTED", "WIFI GOT IP"};
const char const* tcpMasks[] = { "OK", "ERROR", "CONNECT"};
const char const* sendMasks[] = { "SEND OK", "ERROR" };
const char const* indiMask[] = { ">" };
/** GetTokens
* @brief			Get terminal line tokens 
* @param[in]				str: terminal strings;
* @param[out]				tokens: token pointers;
* @param[in]				size: buffer lenght token pointers;
* @return			Integer	total select number tokenst
*/
int GetTokens(char* str, char** tokens, size_t size) 
{
	char* token = NULL;
	char* next_token = str;
	int i = 0;
	while (i < size) {
		token = strtok_s(next_token, "\r\n", &next_token);
		if (!token) break;
		tokens[i++] = token;
	}
	return i;
}
/** CmpTokens
* @brief			Compare tokens with masks
* @param[in]				mask_str: masks list pointer;
* @param[in]				mcnt: masks list pointer length;
* @param[in]				tokens_str: tokens list pointer;
* @param[in]				tcnt: tokens list pointer length;
* @return			Comparison Mask
*/
uint8_t CmpTokens(const char** mask_str, size_t mcnt, char** tokens_str, size_t tcnt)
{
	uint8_t fselect = 0;
	for (int it = 0; it < tcnt; it++) {
		for (int im = 0; im < mcnt; im++) {
			if (strlen(mask_str[im]) < strlen(tokens_str[it])) continue;
			if (!strcmp(tokens_str[it], mask_str[im])) {
				fselect |= (uint8_t)0x1 << im;
				break;
			}
		}
	}
	return fselect;
}
/** ConsoleInputWait
* @brief			Command input string waiter
* @param[out]				buf: input buffer pointer;
* @param[in]				size: input buffer max size;
* @return			
*/
void ConsoleInputWait(char* buf, size_t size)
{
	fgets(buf, size, stdin);
	buf[strcspn(buf, "\n")] = '\0';
}
/** InputHandler
* @brief			Command input handler
* @param[in]				mem: input buffer  
* @param[in]				size: input buffer size  
* @param[out]				method: method type pointer;
* @param[out]				url: url buffer pointer;
* @param[out]				payload: data buffer pointer;
* @return			Total input commands or -1 if exit
*/
int InputHandler(char* mem, size_t size, char** method, char** url, char** payload)
{
	char* pt = mem;
	char* begin = pt;
	char** pparams[3] = { method, url, payload};
	uint32_t psizes[3] = { 5, HOST_MAXLEN + PATH_MAXLEN, TX_PYLO_MAXLEN };
	int ipp = 0;

	printf("Pleas enter request in the format(<REQ><URL>[<BODY>]) or exit:\r\n");
	ConsoleInputWait(mem, size);
	if (strlen(mem) < 4) return 0;
	if (!memcmp(mem, "exit", 4)) return -1;
	while (*pt != '\0') {
		if (*pt == '<') begin = pt + 1;
		else if (*pt == '>') {
			*pt = '\0';
			uint32_t len = pt - begin;
			if (len <= psizes[ipp]) *(pparams[ipp]) = begin;
				else return 0;//incorrect parameters lenght
			if (++ipp == 3) return ipp;
		}
		pt++;
	}
	return ipp;
}
/** ParseUrl
* @brief			Parse Url
* @param[in]				url_str: method type pointer;
* @param[out]				host: extracted Host pointer;
* @param[out]				path: extracted Path pointer;
* @return			1 if Path select or 0 
*/
int ParseUrl(char* url_str, char** host, char** path)
{
	char* p = url_str;
	*host = url_str;
	*path = NULL;
	for (; *p != '\0'; p++) {
		if (*p == '/') {
			if (*(p + 1) == '/'){
				p++;
				continue;
			}
			*p = '\0';
			if (strlen(host) > HOST_MAXLEN) return -1;
			*path = p + 1;
			if (strlen(path) > PATH_MAXLEN) return -1;
			return 1;
		}
	}
	return 0;
}
/** AtHandler
* @brief			AT command handler
* @param[in]				mask[]: buffer pointers comparison masks;
* @param[in]				size: buffer comparison masks size;
* @param[out]				fmask: comparison results pointer;
* @param[in]				cmd_str: command string;
* @param[in]				to: waiting timeout;
* @return			
*/
void AtHandler(char const* mask[], size_t size, uint8_t *fmask, char* cmd_str, uint32_t to)
{
	char* tokens[5];
	if (cmd_str != NULL) {
		ReadPort(respBuf, RESPONSE_MAXLEN);
		if(WritePort(cmd_str, strlen(cmd_str)) <= 0) return;
	}
	if (mask == NULL) return;
	*fmask = 0;
	int res = ReadDataWait(respBuf, RESPONSE_MAXLEN, to);
	if (res > 0) {
		res = GetTokens(respBuf, tokens, sizeof(tokens) / sizeof(tokens[0]));
		*fmask = CmpTokens(mask, size, tokens, res);
	}
}
/** Esp32Init
* @brief			ESP32 initialization
* @return		0 or -1
*/
int Esp32Init()
{
	const char const* cmd[] = { "AT\r\n", "AT+RST\r\n", "ATE0\r\n", "AT+CWMODE=1,0\r\n", "AT+CIPMUX=0\r\n"};
	uint8_t fmask = 0;
	for (int i = 0; i < sizeof(cmd) / sizeof(cmd[0]); i++) {
		AtHandler(defMasks, (sizeof(defMasks) / sizeof(defMasks[0])), &fmask, cmd[i], CMDRESP_TO);
		if (CHECK_MSK(fmask, MSK_OK)) continue;
		return -1;
	}
	return 0;
}
/** ConnectWiFi
* @brief			WiFi connection 
* @return		0 or -1
*/
int ConnectWiFi(char* ssid, char* pwd)
{
	char buf[40];
	uint8_t fmask = 0;

	sprintf_s(buf, sizeof(buf), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
	AtHandler(wifiMasks, (sizeof(wifiMasks)/sizeof(wifiMasks[0])), &fmask, buf, CMDRESP_TO);
	if (CHECK_MSK(fmask, WIFI_CONNECTED_MSK)) return 0;
	return -1;
}
/** SendRequest
* @brief			Send GET or POST request
* @param[in]				method: method type;
* @param[in]				url_str: full url string;
* @param[in]				payload: data pointer;
* @return			Request length or -1
*/
int SendRequest(Method_t method, char* url_str, char* payload)
{
	char* host;
	char* path;
	uint32_t pos = 0;
	char cmd[50];
	uint8_t fmask = 0;
	//Set method
	if (method == GET) {
		pos += sprintf_s(reqBuf, sizeof(reqBuf), "GET /");
	}
	else {
		pos += sprintf_s(reqBuf, sizeof(reqBuf), "POST /");
	}
	ParseUrl(url_str, &host, &path);
	//Set path
	if (path != NULL) {
		pos += sprintf_s(&reqBuf[pos], sizeof(reqBuf) - pos, "%s", path);
	}
	//Set host
	pos += sprintf_s(&reqBuf[pos], sizeof(reqBuf) - pos, " HTTP/1.1\r\nHost: %s\r\n", host);
	//Set Headers and payload
	if (method == GET) {
		pos += sprintf_s(&reqBuf[pos], sizeof(reqBuf) - pos, "Accept: text/html\r\nConnection: close\r\n\r\n");
	}
	else {
		pos += sprintf_s(&reqBuf[pos], sizeof(reqBuf) - pos, "Content-Type: text/html\r\nContent-length: %i\r\n\r\n%s", (int)strlen(payload), payload);
	}
	if (pos > REQUEST_MAXLEN) return -1;
	//Establish TCP Connection
	sprintf_s(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",80\r\n", host);
	AtHandler(tcpMasks, sizeof(tcpMasks) / sizeof(tcpMasks[0]), &fmask, cmd, CMDRESP_TO);
	if (CHECK_MSK(fmask, IP_CONNECTED_MSK)) {
		sprintf_s(cmd, sizeof(cmd), "AT+CIPSEND=%i\r\n", (int)strlen(reqBuf));
		//Send request and payload
		AtHandler(indiMask, sizeof(indiMask) / sizeof(indiMask[0]), &fmask, cmd, CMDRESP_TO);
		if (CHECK_MSK(fmask, MSK_INDICATOR)) {
			AtHandler(sendMasks, sizeof(sendMasks) / sizeof(sendMasks[0]), &fmask, reqBuf, PYLOSEND_TO);
			if (CHECK_MSK(fmask, MSK_SEND_OK)) return pos;
		}
	}
	return -1;
}

int main(void) 
{
	char* method;
	char* url;
	char* data;
	char device[11];
	char mem[HOST_MAXLEN + PATH_MAXLEN + TX_PYLO_MAXLEN];

	//Test_InputHandler();

	printf("Pleas set the serial port in the format (COMx): ");
	ConsoleInputWait(mem, 6);
	sprintf_s(device, sizeof(device), "\\\\.\\%s", mem);

	if (!OpenSerialPort(device, UART_BAUDRATE))
	{ 
		printf("Failed to open serial port\n");
		return -1;
	}
	printf("ESP32 initialize...\n");
	if (Esp32Init() < 0) {
		printf("Error initialize ESP32\n");
		return -1;
	}
	if (ConnectWiFi(_ssid, _pwd) < 0) {
		printf("Error WiFi connection\n");
		return -1;
	}
	printf("ESP32 initialized, WiFi connected.\n");
	while (1) {
		int res = InputHandler(mem, sizeof(mem), &method, &url, &data);
		if (res < 0) break;
		Method_t mtpy;
		if (res == 3 && !memcmp(method, "POST", 4)) {
			mtpy = POST;
		}
		else if (res == 2 && !memcmp(method, "GET", 3)) {
			mtpy = GET;
		}
		else {
			printf("Incorrect parameters\n");
			continue;
		}
		if (SendRequest(mtpy, url, data) > 0) {
			printf("Sending payload...\n");
			res = ReadDataWait(respBuf, RESPONSE_MAXLEN, RCVRESPONSE_TO);
			if (res > 0) {
				printf("\r\nReceive Data:\n%s\n", respBuf);
			}
		}
	}
	CloseSerialPort();

	printf("The application has ended.\n");
	return 1;
}
