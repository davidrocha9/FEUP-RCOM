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

int create_socket(const char* ip_address, int port);
int writeSocket(int socket_fd, char* command, size_t str_size);
int readSocket(FILE *fd, int socket_fd, char* response, size_t str_size);
