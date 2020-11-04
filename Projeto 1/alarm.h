#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_TIMEOUTS 3

struct dataTransfered {
<<<<<<< HEAD
  unsigned int status; // 0 - Sender || 1 - Receiver
=======
  unsigned int status; //0 - Sender || 1 - Receiver
>>>>>>> a05d0580356a02baaa7373f3ec4bb1781e84bc4d
  int ns;
  unsigned int timeouts;
  unsigned int numTries;
  unsigned int alarmFlag;
  struct termios oldtio, newtio;
} data;


void alarm_handler(int signal);
void startAlarm();
void stopAlarm();