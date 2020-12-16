#include "ftp_handler.h"
#include "socket_handler.h"

int ftp_command_response(FILE* fd, const int socket_fd, const char* command, char* response) {
	char buffer[1024]; 
	
	sprintf(buffer, "%s\r\n", command);
	writeSocket(fd, socket_fd, buffer, strlen(buffer));
    readSocket(fd, socket_fd, buffer, sizeof(buffer));

	memcpy(response, buffer, sizeof(buffer));

	return 0;
}

int ftp_command(FILE* fd, int sockfd, char* command) {
    char read_buffer[1024];
    char buffer[1024]; 
	
	sprintf(buffer, "%s\r\n", command);
	writeSocket(fd, sockfd, buffer, strlen(buffer));
    readSocket(fd, sockfd, buffer, sizeof(buffer));

	memcpy(read_buffer, buffer, sizeof(buffer));

	return 0;
}

int ftp_login(FILE* fd, int sockfd, url_info* url) {
    char buffer[1024];

    //USER
    sprintf(buffer, "USER %s\n", url->user);
    ftp_command(fd, sockfd, buffer);

    memset(buffer, 0, sizeof(buffer));

    //PASSWORD
    sprintf(buffer, "PASS %s\n", url->password);
    ftp_command(fd, sockfd, buffer);

    return 0;
}

int ftp_pasv_mode(FILE* fd, int sockfd, url_info* url) {
    char buffer[BUF_SIZE] = "";
    char read_buffer[BUF_SIZE] = "";

    //PASV
    sprintf(buffer, "PASV\n");
    ftp_command_response(fd, sockfd, buffer, read_buffer);

    //PASV RESPONSE
    int ip1, ip2, ip3, ip4, port1, port2;
    
    if (sscanf(read_buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &port1, &port2) < 0) {
        perror("sscanf()\n");
        return 1;
    }

    char ip_address[BUF_SIZE] = "";
    sprintf(ip_address, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    int port_num = port1*256 + port2;
    printf("IP Address: %s, Port: %d\n\n", ip_address, port_num);

    //Creating data socket
    int data_socket;
    if ((data_socket = create_socket(ip_address, port_num)) < 0) {
        perror("create_socket()\n");
        exit(1);
    }

    return data_socket;
}

int ftp_retr(FILE* fd, int sockfd, url_info* url, char* response) {
    char buffer[BUF_SIZE] = "";
    char path[2000] = "";

    sprintf(path, "%s%s", url->url_path, url->filename);
    sprintf(buffer, "RETR %s\n", path);
    ftp_command_response(fd, sockfd, buffer, response);

    int response_code;
    sscanf(response, "%d", &response_code);
    if (response_code != 150) { //Not in good condition
        printf("Error reading file...\n");
        return 1;
    }

    return 0;
}
