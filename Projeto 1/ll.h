#include "alarm.h"
#include "constants.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define MAX_TRIES 3
#define TRANSMITTER 0
#define RECEIVER 1

typedef enum {START, FLAG_RCVD, A_RCVD, C_RCVD, BCC1_RCVD, BCC2_RCVD, DATA_RCVD, END} State;

int llopen(const char* port, int role);

int llclose(int fd, int status);

int sendFrame(int fd, char* packet, int size);

int llwrite(int fd, char* packet, int size);

int llread(int fd, char* packet);

int setStruct(const char* serialPort, int status);