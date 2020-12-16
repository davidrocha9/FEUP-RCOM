#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ftp_handler.h"

int main(int argc, char *argv[]) {

    if (argc != 2) {
        perror("Usage: wrong no. of arguments");
        exit(1);
    }

    url_info url;
    int sockfd;
    int data_socket;

    initialize_struct(&url);

    if (setUpStruct(&url, argv[1])) {
        perror("Error with URL.\n\n");
        exit(1);
    }

    printf("\n\nURL info:\n");
    printf_info(&url);
    printf("-------------------------------------------\n\n");

    if ((sockfd = create_socket(url.ip_address, url.port)) < 0) {
        perror("create_socket()\n");
        exit(1);
    }

    char read_buffer[1024];
    FILE* fd = fdopen(sockfd, "r");
    
    if (readSocket(fd, sockfd, read_buffer, sizeof(read_buffer))) {
		perror("ftp_read()");
		return 1;
	}

    printf("Started ftp connection successfully\n");


    //LOGIN (USER + PASSWORD)
    ftp_login(fd, sockfd, &url);
    printf("Logged in successfully\n");

    //PASSIVE MODE (PASV + PASV RESPONSE)
    data_socket = ftp_pasv_mode(fd, sockfd, &url);
    //ftp_pasv_mode(fd, sockfd, &url, data_socket);


    //RETR
    memset(read_buffer, 0, sizeof(read_buffer));
    if (ftp_retr(fd, sockfd, &url, read_buffer)) {
        fclose(fd);
        close(sockfd);
        close(data_socket);
        exit(1);
    }

    char info[256];
    char path[256];
    int file_size;
    memcpy(info, &read_buffer[44], strlen(read_buffer)-44+1);
    sscanf(info, "%s (%d bytes)", path, &file_size);

    int fd_file;
    if ((fd_file = open(url.filename, O_CREAT | O_WRONLY, 0666)) < 0) {
        perror("open()\n");
        exit(1);
    }

    memset(read_buffer, 0, sizeof(read_buffer));
    int bytes;
    unsigned int totalbytes = 0;
    while((bytes = read(data_socket, read_buffer, sizeof(read_buffer)))) {
        totalbytes += bytes;
        if (bytes < 0) {
            perror("read()\n");
            exit(1);
        }
        if (write(fd_file, read_buffer, bytes) < 0) {
            perror("write()\n");
            exit(1);
        }
    }

    struct stat st;
    stat(url.filename, &st);
    int fsize = st.st_size;
    if (fsize != file_size) {
        printf("File size incorrect. Possible data loss...\n");
    }
    else printf("File size correct...\n");

    memset(read_buffer, 0, sizeof(read_buffer));
    readSocket(fd, sockfd, read_buffer, sizeof(read_buffer));
    printf("Response: %s\n", read_buffer);

    fclose(fd);
    close(sockfd);
    close(data_socket);

    return 0;
}