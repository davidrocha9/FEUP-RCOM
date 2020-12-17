#include "socket_handler.h"

int create_socket(const char* ip_address, int port) {
    int	sockfd;
	struct	sockaddr_in server_addr;
	
	/*server address handling*/
	bzero((char*) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_address);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()\n");
		return 1;
    }

	/*connect to the server*/
	if(connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
		perror("connect()\n");
		return 1;
	}

    return sockfd;
}

int writeSocket(int socket_fd, char* command, size_t str_size) {
    int bytes;

	if ((bytes = write(socket_fd, command, str_size)) <= 0) {
		perror("write()\n");
		return 1;
	}
	return 0;
}

int readSocket(FILE* fd, const int socket_fd, char* response, size_t str_size) {

	do {
		memset(response, 0, str_size);
		response = fgets(response, str_size, fd);
		printf("%s", response);

	} while (!('1' <= response[0] && response[0] <= '5') || response[3] != ' ');

	return 0;
}
