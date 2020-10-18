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

volatile int STOP=FALSE;

char sent[255];
char received[255];
int res, fd;
char buf[255];

void establishLogicGate(){
  // SET
  sent[0] = FLAG;
  sent[1] = A_SET;
  sent[2] = C_SET;
  sent[3] = BCC1;
  sent[4] = FLAG;

  // Sending SET
  while(TRUE){
    res = write(fd,buf,6); 
    printf("SET sent.\n");

    res = read(fd,received,6);
    printf("UA received.\n");

    if(res == 0){
      sleep(1);
    }
    else{
      break;
    }
  }

  printf("UA: ");
  write(STDOUT_FILENO, received, 6);
}


int main(int argc, char** argv)
{
  int c, fd, res;
  struct termios oldtio,newtio;
  char buf[255];
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
  
  size_t len;
  if(gets(buf)==NULL){
      perror("gets");
      exit(-1);
  }
  len = strlen(buf)+1;

  int count = 0;
  while(TRUE){
    res = write(fd,buf,len); 

    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,1);   /* returns after 5 chars have been input */
      buf[res]='\0';               /* so we can printf... */
      //printf(":%s:%d\n", buf, res);
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
  printf("\n");

  close(fd);
  return 0;
}
