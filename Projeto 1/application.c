#include "application.h"

void structSetUp(char* fileName, int packetSize, int fdPort) {
    file_data.fileName = fileName;
    file_data.packetSize = packetSize;
    file_data.serialPort = fdPort;
}

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
    
    file_data.file_fd = fd;
    file_data.fileSize = buf.st_size;
	file_data.fileName = file_name;

    return 0;
}

int controlPacket(int fd, int type){
    unsigned char packet[1024];

    if (type == START)
        packet[0] = 0x02;
    else
        packet[0] = 0x03;

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

    if (llwrite(fd, packet, packetSize) < packetSize){
        perror("LLWRITE START CONTROL PACKET\n");
        exit(1);
    }

    return packetSize;
}

void getName(char* newFileName, unsigned char* message, int size, int index){
    for (int x = 0; x < size; x++){
        newFileName[x] = message[index];
        index++;

        if(x == size - 1){
            message[index] = '\0';
            index++;
        }
    }
}

int readControlPacket(unsigned char* controlPacket){
    unsigned char message[1024];
    int size = llread(file_data.serialPort, controlPacket, message);
    int index, newFilesize = 0, fileSize = 0;
    
    for (index = 1; index < size; index++){
        switch(message[index]){
            case 0x00:
                index += 2;
                fileSize += (message[index] << 24);
                index++;
                fileSize += (message[index] << 16);
                index++;
                fileSize += (message[index] << 8);
                index++;
                fileSize += message[index];
                break;
            case 0x01:
                index++;
                newFilesize = message[index];
                char* newFileName = (char*) malloc(newFilesize + 1);
                index++;

                getName(newFileName, message, newFilesize, index);

                file_data.fdNewFile = open("marega2", O_WRONLY | O_CREAT | O_APPEND, 0664);
                printf("filename: %s\n", newFileName);
                break;
            default:
                break;
        }
    }

    return fileSize;
}

void createPacket(unsigned char* packet, unsigned char* buffer, int size, int packetsSent){
    packet[0] = 0x01;
    packet[1] = packetsSent % 255;
    packet[2] = size / 256;
    packet[3] = size % 256;

    for (int x = 0; x < file_data.packetSize; x++){
        packet[4 + x] = buffer[x];
    }
}

int sendDataPacket(){
    int packetsSent = 0, packetsUnsent = file_data.fileSize/file_data.packetSize;
    unsigned char buffer[file_data.packetSize];
    int size = 0;

    if(file_data.fileSize % file_data.packetSize != 0){
        packetsUnsent++;
    }

    int index = 0;
    while(packetsSent < packetsUnsent){
        if((size = read(file_data.file_fd,buffer, file_data.packetSize)) < 0){
            printf("Error reading file\n");
        }
        index++;
        unsigned char packet[4 + file_data.packetSize];
        createPacket(packet, buffer, size, packetsSent);

        if(llwrite(file_data.serialPort,packet,size + 4) < (size + 4)){
            printf("Error writing data packet to serial port!\n");
            return -1;
        }
        packetsSent++;
    }


    return 0;
}

int sendFile(int fd){
    file_data.serialPort = fd;

    if (controlPacket(fd, START) < 0){
        perror("CONTROL PACKET");
        exit(1);
    }

    if (sendDataPacket() < 0){
        perror("DATA PACKETS");
        exit(1);
    }

    if (controlPacket(fd, END) < 0){
        perror("CONTROL PACKET");
        exit(1);
    }
    return 0;
}

int writeDataToFile(unsigned char* packet){
    
    int informationSize = 256*packet[2] + packet[3];
 
    write(file_data.fdNewFile,packet + 4,informationSize);

    return 0;
}

int readPacket(int fd, unsigned char* endControlPacket){
    unsigned char controlPacket[1024];
    unsigned char buffer[131082], buffer2[131082];

    int size;
    int index = 1, newFilesize = 0, fileSize = 0;

    if(size = llread(fd, buffer2, buffer) > 0){
        switch(buffer[0]){
            case 0x01:
                writeDataToFile(buffer);
                break;
            case 0x03:
                    switch(buffer[index]){
                        case 0x00:
                            index += 2;
                            fileSize += (buffer[index] << 24);
                            index++;
                            fileSize += (buffer[index] << 16);
                            index++;
                            fileSize += (buffer[index] << 8);
                            index++;
                            fileSize += buffer[index];
                            return fileSize;
                        default:
                            break;
                    }
                break;
            default:
                break;
        }
    }

    return 0;
}

int readFile(int fd){
    unsigned char startControlPacket[1024], endControlPacket[1024];
    file_data.serialPort = fd;
    int startSize, finalSize;

    startSize = readControlPacket(startControlPacket);

    while(1){
        if (finalSize = readPacket(fd, endControlPacket)) break; 
    }

    if (startSize != finalSize){
        perror("SIZE DIFFERS\n");
        exit(1);
    }

    close(file_data.fdNewFile);

    return 0;
}
