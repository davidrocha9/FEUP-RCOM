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

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

struct applicationLayer {
  char serialPort[64];   /*Descritor correspondente à porta série*/
  int status;   /*TRANSMITTER | RECEIVER*/
} app;

int llopen(const char* port, int role);
