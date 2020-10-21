#include "ll.h"

char buf[255];
char received[255];
int res, fd;
struct termios oldtio,newtio;

int sendFrame(int fd, unsigned char* packet, int size){
    unsigned int maxSize = 6 + 2*size; // worst case scenario, every byte has to be stuffed (size duplicates) except for the control bytes (6) 
    unsigned char frame[maxSize];
    int frameIndex = 4, index = 0;

    // Control front bytes
    frame[0] = FLAG;
    frame[1] = A_SET;
    if (app.ns == 0)
        frame[2] = BCC_NS0;
    else frame[2] = BCC_NS1;
    frame[3] = (A_SET ^ frame[2]);

    unsigned char bcc2 = 0x00;

	for (int i = 0; i < size; i++)
		bcc2 ^= packet[i];

    while(index < size){
        if (packet[index] == FLAG || packet[index] == ESC){
            frame[4+index] = ESC;
            frame[4+index+1] = packet[index] ^ STUFFING;
            index++;
        }
        else frame[4+index] = packet[index];

        index++;
    }

    // Control back bytes
    if (bcc2 == FLAG || bcc2 == ESC){
        frame[4+index] = ESC;
        frame[4+index+1] = bcc2 ^ STUFFING;
        index++;
    }
    else frame[index] = bcc2;
    index++;

    frame[index] = FLAG;

    write(fd, frame, size);

    printf("Frame size: %d\n", index);
    return index;  
}

int llwrite(int fd, unsigned char* packet, int size){
    printf("vou começar o envio\n");
    sendFrame(fd, packet, size);
    return 0;
}

int llread(int fd, unsigned char* packet){
    res = read(fd,buf,15);
    printf("%s\n", buf);
}

int setStruct(const char* serialPort, int status){
    fd = open(serialPort, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(serialPort); exit(-1); }

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
    newtio.c_cc[VMIN]     = 5;

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    strcpy(app.serialPort, serialPort);
    app.status = status;
    app.ns = 0;

    return fd;
}

int llopen(const char* serialPort, int status){
    int fd = setStruct(serialPort, status);
    
    switch(status){
        case 0:
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
            break;
        case 1:
            printf("Waiting for SET...\n");
            res = read(fd,received,5);
            printf("Received SET. Checking values...\n");

            if (received[0] != FLAG || received[4] != FLAG){
                printf("FLAG error\n");
                return 1;
            }
            else if (received[1] != A_SET){
                printf("A_SET error\n");
                return 1;
            }
            else if (received[2] != C_SET){
                printf("C_SET error\n");
                return 1;
            }
            else if (received[3] != BCC1){
                printf("BCC1 error\n");
                return 1;
            }
            else{
                printf("SET is valid\n");
            }

            buf[0] = FLAG;
            buf[1] = A_UA;
            buf[2] = C_UA;
            buf[3] = BCC2;
            buf[4] = FLAG;
            res = write(fd,buf,5);  
            printf("UA Sent.\n");

            break;
        default:
            printf("Status error!\n");
            return -1;
    }
    
    return fd;
}

int llclose(int fd, int status){
    switch(status){
        case 0:
            buf[0] = FLAG;
            buf[1] = A_SET;
            buf[2] = C_DISC;
            buf[3] = BCC_DISC;
            buf[4] = FLAG;  
            res = write(fd,buf,5);
            printf("DISC sent.\n");

            // DISC Handling
            printf("Waiting for DISC...\n");

            res = read(fd,received,5);
            printf("Received DISC. Checking values...\n");

            if (received[0] != FLAG || received[4] != FLAG){
                printf("FLAG error\n");
                return 1;
            }
            else if (received[1] != A_SET){
                printf("A_SET error\n");
                return 1;
            }
            else if (received[2] != C_DISC){
                printf("C_DISC error\n");
                return 1;
            }
            else if (received[3] != BCC_DISC){
                printf("BCC_DISC error\n");
                return 1;
            }
            else{
                printf("DISC is valid\n");
            }

            buf[0] = FLAG;
            buf[1] = A_UA;
            buf[2] = C_UA;
            buf[3] = BCC2;
            buf[4] = FLAG;
            res = write(fd,buf,5);  
            printf("UA Sent.\n"); 
            break;
        case 1:
            printf("Waiting for DISC...\n");
            res = read(fd,received,5);
            printf("Received DISC. Checking values...\n");

            if (received[0] != FLAG || received[4] != FLAG){
                printf("FLAG error\n");
                return 1;
            }
            else if (received[1] != A_SET){
                printf("A_SET error\n");
                return 1;
            }
            else if (received[2] != C_DISC){
                printf("C_DISC error\n");
                return 1;
            }
            else if (received[3] != BCC_DISC){
                printf("BCC_DISC error\n");
                return 1;
            }
            else{
                printf("DISC is valid\n");
            }

            buf[0] = FLAG;
            buf[1] = A_SET;
            buf[2] = C_DISC;
            buf[3] = BCC_DISC;
            buf[4] = FLAG;  
            res = write(fd,buf,5);
            printf("DISC sent.\n");  
            
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
            break;
        default:
            printf("Status error!\n");
            return -1;
    }

    return fd;
}

