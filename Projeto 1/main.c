/*Non-Canonical Input Processing*/

#include "application.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
  int fd;
  
  struct timeval start, end;

  gettimeofday(&start, NULL);

  if ( (argc < 3) || 
        ((strcmp("/dev/ttyS10", argv[1])!=0) && 
        (strcmp("/dev/ttyS11", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort Status\n\tex: nserial /dev/ttyS0 0 \n");
    exit(1);
  }

/*
  Open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
*/
  int index = atoi(argv[2]);
  int packetSize = 1024;
  char* baudrateNo = "B38400";

  if (index == 0 && argc > 4){
    baudrateNo = argv[4];
  }
  if (index == 1 && argc > 3) {
    baudrateNo = argv[3];
  }

  if (argc > 5)
    packetSize = atoi(argv[5]);

  if (index == 0 && argc < 4){
    printf("Usage for sender:\tnserial SerialPort Status FileName\n\tex: nserial /dev/ttyS1 0 pinguim.gif\n");
    exit(1);
  }

  if ((fd = llopen(argv[1], index, baudrateNo)) == -1) {
		perror("LLOPEN");
		return -1;
	}

  structSetUp(argv[3], packetSize, fd);

  if (index == 0){
    readFileData(argv[3]);  
  }


  printf("\n\n");

  switch(index){
    case 0:
      if(sendFile(fd) == -1) {
        perror("SENDING FILE");
        return -1;
		  }
      break;
    case 1:
      if(readFile(fd) == -1) {
        perror("RECEIVING FILE");
        return -1;
		  }
      break;
  }

  printf("\n\n");

  if (llclose(fd, index) == -1) {
		perror("LLCLOSE");
		return -1;
	}

  close(fd);

  gettimeofday(&end, NULL);
  
  double time_taken = end.tv_sec + end.tv_usec / 1e6 - start.tv_sec - start.tv_usec / 1e6; 

  printf("Program took %f seconds to transfer file\n", time_taken);
  printf("Packets per iteration: %d\n", packetSize);
  printf("Baudrate: %s\n", baudrateNo);


  return 0;
}
