#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "url_parser.h"
#include "socket_handler.h"

typedef struct ftp_t {
    int control_socket_fd;
    int data_socket_fd;
} ftp_t;

int ftp_command(FILE* fd, int sockfd, char* command);

int ftp_command_response(FILE* fd, const int socket_fd, const char* command, char* response);

int ftp_login(FILE* fd, int sockfd, url_info* url);

int ftp_pasv_mode(FILE* fd, int sockfd, url_info* url);

int ftp_retr(FILE* fd, int sockfd, url_info* url, char* response);
