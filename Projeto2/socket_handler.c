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

/*int writeSocket(int socket_fd, char* str, size_t str_size) {
    int bytes;

    // Write a string to the server
	if ((bytes = write(socket_fd, str, str_size)) <= 0) {
		perror("Write to socket\n");
		return 1;
	}

	printf("Bytes written to server: %d\nInfo: %s\n", bytes, str);

	return 0;
}

int readSocket(int socket_fd, char* str, size_t str_size) {
    FILE* fp = fdopen(socket_fd, "r");

	do {
		memset(str, 0, str_size);
		str = fgets(str, str_size, fp);
		printf("%s", str);

	} while (!('1' <= str[0] && str[0] <= '5') || str[3] != ' ');

	return 0;
}*/

int writeSocket(FILE* fd, const int socket_fd, const char* str, const size_t str_size) {
    int bytes;

	printf("%d\n", socket_fd);
    // Write a string to the server
	if ((bytes = write(socket_fd, str, str_size)) <= 0) {
		perror("Write to socket\n");
		return 1;
	}

	printf("Bytes written to server: %d\nInfo: %s\n", bytes, str);

	return 0;
}

int readSocket(FILE* fd, const int socket_fd, char* str, size_t str_size) {
	do {
		memset(str, 0, str_size);
		str = fgets(str, str_size, fd);
		printf("%s", str);

	} while (!('1' <= str[0] && str[0] <= '5') || str[3] != ' ');

	return 0;
}
