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
    printf("%s", newFileName);
    sleep(4);
}

int readControlPacket(unsigned char* controlPacket){
    unsigned char message[1024];
    int size = llread(file_data.serialPort, controlPacket, message);
    int index = 0, newFilesize = 0, fileSize = 0;
    
    if (message[index] == 0x02){
        index += 7;
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

<<<<<<< HEAD
                file_data.fdNewFile = open(newFileName, O_WRONLY | O_CREAT | O_APPEND, 0664);
=======
                file_data.fdNewFile = open("marega2", O_WRONLY | O_CREAT | O_APPEND, 0664);
>>>>>>> a05d0580356a02baaa7373f3ec4bb1781e84bc4d
                break;
            default:
                break;
        }
    }

    return size;
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
    int packetsSent = 0, packetsUnsent = file_data.fileSize / file_data.packetSize;
    unsigned char buffer[file_data.packetSize + 4];
    int size = 0;

    if(file_data.fileSize % file_data.packetSize != 0){
        packetsUnsent++;
    }

    int index = 0;
    while(packetsSent < packetsUnsent){
<<<<<<< HEAD
        if((size = read(file_data.file_fd,buffer, file_data.packetSize)) < 0){
=======
        if((size = read(file_data.file_fd, buffer, file_data.packetSize)) < 0){
>>>>>>> a05d0580356a02baaa7373f3ec4bb1781e84bc4d
            printf("Error reading file\n");
        }
        index++;
        unsigned char packet[4 + file_data.packetSize];
        createPacket(packet, buffer, size, packetsSent);

        printf("Iteracao %d\n", index);

        if(llwrite(file_data.serialPort, packet, size + 4) < (size + 4)){
            printf("Error writing data packet to serial port!\n");
            return -1;
        }
        printf("\n");
        packetsSent++;
    }


    return 0;
}

int sendFile(int fd){
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
 
<<<<<<< HEAD
    write(file_data.fdNewFile,packet + 4,informationSize);
=======
    write(file_data.fdNewFile, packet + 4, informationSize);
>>>>>>> a05d0580356a02baaa7373f3ec4bb1781e84bc4d

    return 0;
}

int readPacket(int fd){
    unsigned char buffer[MAX_BUFFER_SIZE], buffer2[MAX_BUFFER_SIZE];

    if(llread(fd, buffer2, buffer) > 0){
        switch(buffer[0]){
            case 0x01:
                writeDataToFile(buffer);
                break;
            case 0x03:
                return 1;
            default:
                break;
        }
    }

    return 0;
}

int readFile(int fd){
    unsigned char controlPacket[1024];

    readControlPacket(controlPacket);

    while(1){
        if (readPacket(fd)) break; 
    }

    close(file_data.fdNewFile);

    return 0;
}