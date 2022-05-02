#include "utils.h"
#include "main.h"
#include "config.h"

typedef void (*Display_t)(char*);
static void display_request(char* src) 
{
	printf("Request type: %s\n", src);
}
static void display_url(char* src) 
{
	char* host;
	char* path;
	int res = ParseUrl(src, &host, &path);
	printf("Request Url: %s\n", src);
	printf("Request Host: %s\n", host);
	if (res > 0) {
		printf("Request Path: %s\n", path);
	}
	else {
		printf("Request Path: NULL\n");
	}
}
static void display_data(char* src)
{
	printf("Request Data: %s\n", src);
}

void Test_InputHandler()
{
	char* buffs[3];

	char mem[HOST_MAXLEN + PATH_MAXLEN + TX_PYLO_MAXLEN];
	Display_t foo[3] = { display_request , display_url , display_data };

	int res = InputHandler(mem, sizeof(mem), &(buffs[0]), &(buffs[1]), &(buffs[2]));
	if(res < 0) {
		printf("Entered \'exit\' command");
	}
	else if (res > 0 && res <= 3) {
		for (int i = 0; i < res; i++) {
			foo[i](buffs[i]);
		}
	}
}
