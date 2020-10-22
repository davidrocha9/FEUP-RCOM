#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "constants.h"
#include "alarm.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define MAX_TRIES 3
#define TRANSMITTER 0
#define RECEIVER 1

struct applicationLayer {
  char serialPort[64];   /*Descritor correspondente à porta série*/
  unsigned int status;   /*TRANSMITTER | RECEIVER*/
  int ns;                /*sequence number*/
  unsigned int timeouts;
  unsigned int numTries;
  unsigned int alarmFlag;
  struct termios oldtio, newtio;
} app;

extern struct app;

int llopen(const char* port, int role);

int llclose(int fd, int status);

int sendFrame(int fd, unsigned char* packet, int size);

int llwrite(int fd, unsigned char* packet, int size);

int llread(int fd, unsigned char* packet);

int setStruct(const char* serialPort, int status);
