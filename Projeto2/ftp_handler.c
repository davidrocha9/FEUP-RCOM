#include "ftp_handler.h"
#include "socket_handler.h"

int ftp_command(FILE* fd, int sockfd, char* command) {
    char read_buffer[BUF_SIZE];
    char buffer[BUF_SIZE]; 
	
	sprintf(buffer, "%s\r\n", command);
	if (writeSocket(sockfd, buffer, strlen(buffer))) {
        perror("write to socket\n");
        return 1;
    }

    if (readSocket(fd, sockfd, buffer, sizeof(buffer))) {
        perror("read from socket\n");
        return 1;
    }

	memcpy(read_buffer, buffer, sizeof(buffer));

	return 0;
}

int ftp_command_response(FILE* fd, int sockfd, char* command, char* response) {
	char buffer[BUF_SIZE]; 
	
	sprintf(buffer, "%s\r\n", command);
	if (writeSocket(sockfd, buffer, strlen(buffer))) {
        perror("write to socket\n");
        return 1;
    }

    if (readSocket(fd, sockfd, buffer, sizeof(buffer))) {
        perror("read from socket\n");
        return 1;
    }

	memcpy(response, buffer, sizeof(buffer));

	return 0;
}

int ftp_login(FILE* fd, int sockfd, url_info* url) {
    char buffer[BUF_SIZE];
    char read_buffer[BUF_SIZE] = "";

    //USER
    sprintf(buffer, "USER %s\n", url->user);
    if (ftp_command_response(fd, sockfd, buffer, read_buffer)) {
        perror("user()\n");
        return 1;
    }

    int response_code;
    sscanf(read_buffer, "%d", &response_code);
    if (response_code != 230 && response_code != 331) {
        printf("Error user()\n");
        return 1;
    }

    memset(buffer, 0, sizeof(buffer));
    memset(read_buffer, 0, sizeof(read_buffer));

    //PASSWORD
    sprintf(buffer, "PASS %s\n", url->password);
    if (ftp_command_response(fd, sockfd, buffer, read_buffer)) {
        perror("pass()\n");
        return 1;
    }

    sscanf(read_buffer, "%d", &response_code);
    if (response_code == 530) {
        printf("Error pass()\n");
        return 1;
    }

    return 0;
}

int ftp_pasv_mode(FILE* fd, int sockfd, url_info* url, int* data_socket) {
    char buffer[BUF_SIZE] = "";
    char read_buffer[BUF_SIZE] = "";

    //PASV
    sprintf(buffer, "PASV\n");
    if (ftp_command_response(fd, sockfd, buffer, read_buffer)) {
        perror("pasv()\n");
        return 1;
    }

    int response_code;
    sscanf(read_buffer, "%d", &response_code);
    if (response_code != 227) {
        printf("Error pasv()\n");
        return 1;
    }

    //PASV RESPONSE
    int ip1, ip2, ip3, ip4, port1, port2;
    
    if (sscanf(read_buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &port1, &port2) < 0) {
        perror("sscanf()\n");
        return 1;
    }

    char ip_address[BUF_SIZE] = "";
    sprintf(ip_address, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    int port_num = port1*256 + port2;
    printf("IP Address: %s, Port: %d\n", ip_address, port_num);

    //Creating data socket
    if ((*data_socket = create_socket(ip_address, port_num)) < 0) {
        perror("create_socket()\n");
        exit(1);
    }

    return 0;
}

int ftp_retr(FILE* fd, int sockfd, url_info* url, char* response) {
    char buffer[BUF_SIZE] = "";
    char path[2000] = "";

    sprintf(path, "%s%s", url->url_path, url->filename);
    sprintf(buffer, "RETR %s\n", path);
    if (ftp_command_response(fd, sockfd, buffer, response)) {
        perror("retr()\n");
        return 1;
    }

    int response_code;
    sscanf(response, "%d", &response_code);
    if (response_code != 150) {
        printf("Error retr()...\n");
        return 1;
    }

    return 0;
}

int ftp_quit(FILE* fd, int sockfd, url_info* url) {
    char buffer[BUF_SIZE] = "";
    char read_buffer[BUF_SIZE] = "";

    sprintf(buffer, "QUIT\n");
    if (ftp_command_response(fd, sockfd, buffer, read_buffer)) {
        perror("quit()\n");
        return 1;
    }

    int response_code;
    sscanf(read_buffer, "%d", &response_code);
    if (response_code == 221) {
        printf("Error pass()\n");
        return 1;
    }
    
    return 0;
}
