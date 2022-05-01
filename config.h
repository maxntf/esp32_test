#ifndef __CONFIG_H
#define __CONFIG_H
//User configure params
#define UART_BAUDRATE	9600
#define WIFI_SSID "user"
#define WIFI_PWD "admin"
//Developer configure params
//port
#define SERIAL_RX_TO	10				//Milliseconds
#define SERIAL_TX_TO	100				//Milliseconds
//at command timeouts
#define CMDRESP_TO		7000U			//Milliseconds
#define PYLOSEND_TO		7000U			//Milliseconds
#define RCVRESPONSE_TO	7000U			//Milliseconds
//data length
#define TX_PYLO_MAXLEN		256U		//bytes
#define RX_PYLO_MAXLEN		1000U		//bytes

#endif/* __CONFIG_H */