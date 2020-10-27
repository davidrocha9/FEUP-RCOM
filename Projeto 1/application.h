#include "ll.h"

#define START 2
#define END 3

int readFileData(char* file_name);

int sendFile(int fd);

int readFile(int fd);

struct fileData {
    char* fileName;
    int fileSize;
    int file_fd;
    int fdNewFile;
    int serialPort;
} file_data;