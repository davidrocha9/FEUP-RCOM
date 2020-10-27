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

    if (llwrite(fd, packet, packetSize) < 0){
        perror("LLWRITE START CONTROL PACKET\n");
        exit(1);
    }

    return 0;
}

int readControlPacket(unsigned char* controlPacket){
    unsigned char message[1024];
    int size = llread(file_data.serialPort, controlPacket, message);
    char* newFile;
    int index = 0, newFilesize = 0;
    
    if (message[index] == 0x02){ //START
        index += 7;
        if (message[index] == 0x01){
            index++;
            newFile = (char*) malloc(message[index]+1);
            newFilesize = message[index];
            index++;

            for (int x = 0; x < newFilesize; x++){
                newFile[x] = message[index];
                index++;

                if(x == newFilesize - 1){
                    message[index] = '\0';
                    index++;
                }
            }

            file_data.fdNewFile = open("marega2",O_WRONLY | O_CREAT | O_APPEND, 0664);
        }
    }

    return size;
}

int sendDataPacket(){
    int numPacketsSent = 0;
    int numPacketsToSend = file_data.fileSize/1024;            // numero máximo de de octetos num packet
    unsigned char buffer[1024];
    int bytesRead = 0;
    int length = 0;

    if(file_data.fileSize%1024 != 0){
        numPacketsToSend++;
    }

    while(numPacketsSent < numPacketsToSend){
        if((bytesRead = read(file_data.file_fd,buffer,1024)) < 0){
            printf("Error reading file\n");
        }

        printf("%d\n", bytesRead);

        unsigned char packet[4+1024];
        packet[0] = 0x01;
        packet[1] = numPacketsSent % 255;
        packet[2] = bytesRead / 256;
        packet[3] = bytesRead % 256;

        for (int x = 0; x < 1024; x++){
            packet[4 + x] = buffer[x];
        }
        length = bytesRead + 4;
        if(llwrite(file_data.serialPort,packet,length) < length){
            printf("Error writing data packet to serial port!\n");
            return -1;
        }
        numPacketsSent++;
    }


    return 0;
}

int sendFile(int fd){
    file_data.serialPort = fd;
    controlPacket(fd, START);

    sendDataPacket();

    controlPacket(fd, END);
    
    return 0;
}

int processDataPackets(unsigned char* packet){
    
    int informationSize = 256*packet[2]+packet[3];
 
    write(file_data.fdNewFile,packet+4,informationSize);

    return 0;
}

int readFile(int fd){
    unsigned char controlPacket[1024];
    unsigned char buffer[2048], buffer2[2048];
    file_data.serialPort = fd;
    int stop = 0;

    int size = readControlPacket(controlPacket);

    while(!stop){
        if(llread(fd,buffer2, buffer) != 0){
            if(buffer[0] == 0x01){
                processDataPackets(buffer);
            }
            else if(buffer[0] == 0x03){
                stop = 1;
            }
        }
    }

    close(file_data.fdNewFile);

    return 0;
}