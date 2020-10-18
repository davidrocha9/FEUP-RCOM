/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define A_SET 0x03
#define C_SET 0x03
#define BCC1 (A_SET ^ C_SET)

#define A_UA 0x01
#define C_UA 0x07
#define BCC2 (A_UA ^ C_UA)

volatile int STOP=FALSE;

char sent[255];
char received[255];
int res, fd;
char buf[255];

int establishLogicGate(){
  // Preparation to send SET
  buf[0] = FLAG;
  buf[1] = A_SET;
  buf[2] = C_SET;
  buf[3] = BCC1;
  buf[4] = FLAG;
  res = write(fd,buf,5);
  printf("SET sent.\n");

  // UA Handling
  printf("Waiting for UA...\n");
  res = read(fd,received,5);
  printf("Received UA. Checking values...\n");

  if (received[0] != FLAG || received[4] != FLAG){
    printf("FLAG error\n");
    return 1;
  }
  else if (received[1] != A_UA){
    printf("A_UA error\n");
    return 1;
  }
  else if (received[2] != C_UA){
    printf("C_UA error\n");
    return 1;
  }
  else if (received[3] != BCC2){
    printf("BCC2 error\n");
    return 1;
  }
  else{
    printf("UA is valid\n");
  }

  return 0;
}


int main(int argc, char** argv)
{
  int c;
  struct termios oldtio,newtio;
  int i, sum = 0, speed = 0;
  
  if ( (argc < 2) || 
        ((strcmp("/dev/ttyS0", argv[1])!=0) && 
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }


/*
  Open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
*/


  fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd <0) {perror(argv[1]); exit(-1); }

  if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */



/* 
  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
  leitura do(s) pr�ximo(s) caracter(es)
*/



  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  establishLogicGate();
  
  /*size_t len;
  if(gets(buf)==NULL){
      perror("gets");
      exit(-1);
  }
  len = strlen(buf)+1;

  int count = 0;
  while(TRUE){
    res = write(fd,buf,len); 

    while (STOP==FALSE) {
      res = read(fd,buf,1);
      buf[res]='\0';

      received[count]=buf[0];
      count++;
      if (buf[0]=='\0') STOP=TRUE;
    }

    if(res == 0){
      sleep(1);
    }
    else{
      break;
    }
  }

  printf("Received: ");
  printf("%s", received);
  printf("\n");*/

  close(fd);
  return 0;
}
