#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_SIZE 256
#define BUF_SIZE 1024

typedef struct url_info {
	char user[MAX_SIZE]; 
	char password[MAX_SIZE]; 
	char host_name[MAX_SIZE]; 
	char ip_address[MAX_SIZE]; 
	char url_path[BUF_SIZE]; 
	char filename[MAX_SIZE]; 
	int port; 
} url_info;

void initialize_struct(url_info* url);
int setUpStruct(url_info* url, const char* ip_str);

int get_url_path(const char* ip_str, char* url_path, char* filename);
char* get_str_before_char(const char* str, const char chr);

void printf_info(url_info* url);
