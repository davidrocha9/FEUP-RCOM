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

int readSET(int fd); 

int readUA(int fd); 

int readDISC(int fd);

int checkSucess(int fd, unsigned char* packet);

void stateMachine(State *state, unsigned char byte);

int readFrame(int fd, unsigned char* packet);

int destuff(unsigned char* packet, unsigned char* destuffed, int size, unsigned char* message);

int verifyPacket (unsigned char* destuffedFrame, int size, unsigned char* message);

int buildResponse(unsigned char* response, char* flag);

int llopen(const char* port, int role, const char* baudrate);

int llclose(int fd, int status);

int sendFrame(int fd, unsigned char* packet, int size);

int llwrite(int fd, unsigned char* packet, int size);

int llread(int fd, unsigned char* packet, unsigned char* message);

int setStruct(const char* serialPort, int status, const char* baudrate);