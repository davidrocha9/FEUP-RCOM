/*Non-Canonical Input Processing*/

#include "ll.h"

volatile int STOP=FALSE;

char sent[255];
char received[255];
int res, fd;
char buf[255];

int main(int argc, char** argv)
{
  int c;
  struct termios oldtio, newtio;
  int i, sum = 0, speed = 0;
  
  if ( (argc < 3) || 
        ((strcmp("/dev/ttyS10", argv[1])!=0) && 
        (strcmp("/dev/ttyS11", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort Status\n\tex: nserial /dev/ttyS1 0\n");
    exit(1);
  }

/*
  Open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
*/
  int index = atoi(argv[2]);

  if ((fd = llopen(argv[1], atoi(argv[2]))) == -1) {
		perror("LLOPEN");
		return -1;
	}

  printf("\n\n");
  
  char message[256] = "ola";
  char buffer[256] = "";
  if (index == 0){
    if (llwrite(fd, message, strlen(message)) == -1) {
		  perror("LLWRITE");
		  return -1;
	  }
  }
  else{
    if (llread(fd, buffer) == -1) {
		  perror("LLREAD");
		  return -1;
	  }
  }

  printf("\n\n");

  char message2[256] = "adeus";
  char buffer2[256] = "";
  if (index == 0){
    if (llwrite(fd, message2, strlen(message2)) == -1) {
		  perror("LLWRITE");
		  return -1;
	  }
  }
  else{
    if (llread(fd, buffer2) == -1) {
		  perror("LLREAD");
		  return -1;
	  }
  }

  printf("\n\n");

  char message3[256] = "sryy";
  char buffer3[256] = "";
  if (index == 0){
    if (llwrite(fd, message3, strlen(message3)) == -1) {
		  perror("LLWRITE");
		  return -1;
	  }
  }
  else{
    if (llread(fd, buffer3) == -1) {
		  perror("LLREAD");
		  return -1;
	  }
  }

  printf("\n\n");

  if (llclose(fd, atoi(argv[2])) == -1) {
		perror("LLCLOSE");
		return -1;
	}

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
