#include "ll.h"

#define START 2
#define END 3

struct fileData {
    char* fileName;
    int fileSize;
    int file_fd;
    int fdNewFile;
    int serialPort;
    int packetSize;
} file_data;

int structSetUp(char* fileName, int packetSize, int fdPort);

int readFileData(char* file_name);

int controlPacket(int fd, int type);

void getName(char* newFileName, char* message, int size, int index);

int readControlPacket(unsigned char* controlPacket);

void createPacket(unsigned char* packet, unsigned char* buffer, int size, int packetsSent);

int sendDataPacket();

int sendFile(int fd);

int writeDataToFile(unsigned char* packet);

int readPacket(int fd);

int readFile(int fd);
