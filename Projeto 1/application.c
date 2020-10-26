#include "application.h"

int readFileData(char* file_name){
    struct stat buf;
    int fd = open(file_name, O_RDONLY);
    if(fd == -1){
        printf("Error opening file!\n");
        return -1;
    }

    if (fstat(fd, &buf) == -1) {
		printf("ERROR: fstat");
		return -1;
	}

    file_data.fd = fd;
    file_data.fileSize = buf.st_size;
	file_data.fileName = file_name;
    
    return 0;
}

int controlPacket(int fd, int type){
    char packet[1024];

    packet[0] = 0x02;
    packet[1] = 0x00;
    packet[2] = sizeof(file_data.fileSize);
    packet[3] = (file_data.fileSize >> 24) & 0xFF;
    packet[4] = (file_data.fileSize >> 16) & 0xFF;
    packet[5] = (file_data.fileSize >> 8) & 0xFF;
    packet[6] = file_data.fileSize & 0xFF;

    packet[7] = 0x01;
    packet[8] = strlen(file_data.fileName);
    for (int x = 0; x < strlen(file_data.fileName); x++){
        packet[9+x] = file_data.fileName[x];
    }

    int packetSize = strlen(file_data.fileName) + 9 * sizeof(unsigned char);

    char david[3] = "abc";

    //write(STDOUT_FILENO, packet, packetSize);
    llwrite(fd, packet, packetSize);

    return 0;
}

int sendFile(int fd){
    controlPacket(fd, START);

    return 0;
}

int readFile(int fd){
    char controlPacket[1024];

    int size = llread(fd, controlPacket);
    
    write(STDOUT_FILENO, controlPacket, size);

    return 0;
}