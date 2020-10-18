
/*#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    size_t len;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { 
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;


    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   
    newtio.c_cc[VMIN]     = 1;   



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");
    
    if(gets(buf)==NULL){
        perror("gets");
        exit(-1);
    }

    len = strlen(buf)+1;

    char receivedString[255];

    while(TRUE){
      printf("dentro\n");
      res = write(fd,buf,len);
      printf("dentro2\n");
      sleep(2);
      res = read(fd,receivedString,1);  
      printf("dentro3\n");
      if(res == 0){
        sleep(1);
      }
      else{
        break;
      }
    }


    sleep(1);   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}
*/

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
#define MODEMDEVICE "/dev/ttyS11"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define A_SEND 0x03
#define C_SET 0x03
#define BCC1 (A_SEND ^ C_SET)

#define A_Recieve 0x01
#define C_UA 0x07
#define BCC2 (A_Recieve ^ C_UA)

#define NUM_TRIES 3

//int UA_received = FALSE;
volatile int STOP=FALSE;
volatile int SUCESS=TRUE;

int tries = 0;


char str[255]; //SET array
char stri[255]; //UA array
//char UA[255]; //UA array
int fd;
int UA_read = FALSE;

int res;

void alarmHandler(){
  printf("UA Was not sent. Retrying.\n");
  printf("TODO: implement reset function\n");
  exit(0);
}


int main(int argc, char** argv)
{
    int c;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
    signal(SIGALRM, alarmHandler);
    
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

    /*testing*/
    
    sprintf((str + 0) , "%c", (char) FLAG);
    sprintf((str + 1) , "%c", (char) A_SEND);
    sprintf((str + 2) , "%c", (char) C_SET);
    sprintf((str + 3) , "%c", (char) BCC1);
    sprintf((str + 4) , "%c", (char) FLAG);

    
    int n = 0;
    
    int a = 0;

    size_t len;
    if(gets(buf)==NULL){
        perror("gets");
        exit(-1);
    }
    len = strlen(buf)+1;

    //End of "Sending Set"

    int count = 0;
    while(TRUE){
      res = write(fd,buf,len); 
      
      while (STOP==FALSE) {       /* loop for input */
        res = read(fd,buf,1);   /* returns after 5 chars have been input */
        buf[res]='\0';               /* so we can printf... */
        //printf(":%s:%d\n", buf, res);
        stri[count]=buf[0];
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
    //res = read(fd,stri,6);   /* returns after 5 chars have been input */
    printf("Received: ");
    printf("%s", stri);
    printf("\n");

    close(fd);
    return 0;
}
