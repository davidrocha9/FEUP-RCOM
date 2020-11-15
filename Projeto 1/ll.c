#include "ll.h"

struct termios oldtio, newtio;

int indexIteration = 0;

speed_t checkBaudrate(long br){
    switch (br){
        case 0xB0:
            return B0;
        case 0xB50:
            return B50;
        case 0xB75:
            return B75;
        case 0xB110:
            return B110;
        case 0xB134:
            return B134;
        case 0xB150:
            return B150;
        case 0xB200:
            return B200;
        case 0xB300:
            return B300;
        case 0xB600:
            return B600;
        case 0xB1200:
            return B1200;
        case 0xB1800:
            return B1800;
        case 0xB2400:
            return B2400;
        case 0xB4800:
            return B4800;
        case 0xB9600:
            return B9600;
        case 0xB19200:
            return B19200;
        case 0xB38400:
            return B38400;
        case 0xB57600:
            return B57600;
        case 0xB115200:
            return B115200;
        default:
            printf("Bad baudrate value. Using default (B38400)");
            return B38400;
    }
}   

int readSET(int fd) {
    unsigned char received[255];
    read(fd, received, 5);
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
    else if (received[3] != BCC_SET){
        printf("BCC_SET error\n");
        return 1;
    }
    else{
        printf("SET is valid\n");
    }

    return 0;
}

int readUA(int fd) {
    unsigned char received[255];
    read(fd, received, 5);
    printf("Received SET. Checking values...\n");

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
    else if (received[3] != BCC_UA){
        printf("BCC_UA error\n");
        return 1;
    }
    else{
        printf("SET is valid\n");
    }

    return 0;
}

int readDISC(int fd) {
    unsigned char received[255];
    read(fd, received, 5);
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
    return 0;
}

int stopConnection(int fd) {
    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }
    close(fd);
    printf("\nENDING DATA TRANSFER\n");
    return 0;
}

int sendFrame(int fd, unsigned char* packet, int size){
    unsigned int maxSize = 6 + 2*size; // worst case scenario, every byte has to be stuffed (size duplicates) except for the control bytes (6) 
    unsigned char frame[maxSize];
    int index = 0;

    // Control front bytes
    frame[0] = FLAG;
    frame[1] = A_SET;
    if (data.ns == 0)
        frame[2] = C_NS0;
    else
        frame[2] = C_NS1;
    frame[3] = (A_SET ^ frame[2]);

    unsigned char bcc2 = 0x00;

    for (int i = 0; i < size; i++)
        bcc2 ^= packet[i];

    for (int x = 0; x < size; x++){
        if (packet[x] == FLAG || packet[x] == ESC){
            frame[index + 4] = ESC;
            frame[index + 4 + 1] = packet[x] ^ STUFFING;
            index++;
        }
        else frame[index + 4] = packet[x];

        index++;
    }

    // Control back bytes
    if (bcc2 == FLAG || bcc2 == ESC){
        frame[4 + index] = ESC;
        frame[4 + index + 1] = bcc2 ^ STUFFING;
        index++;
    }
    else frame[4 + index] = bcc2;
    index++;
    frame[4 + index] = FLAG;
    

    int totalSize = index + 4 + 1;
    write(fd, frame, totalSize);

    return totalSize;  
}

void responseStateMachine(State* currentState, unsigned char byte, unsigned char* controlByte){
    switch(*currentState){
        case START:
            if(byte == FLAG){    //flag
                *currentState = FLAG_RCVD;
            }
            break;
        case FLAG_RCVD:
            if(byte == A_SET){   //acknowlegement
                *currentState = A_RCVD;
            }
            else if(byte != FLAG){
              *currentState = START;
            }
            break;
        case A_RCVD:
            if(byte == CONTROL_BYTE_SET || byte == CONTROL_BYTE_DISC || byte == CONTROL_BYTE_UA || byte == CONTROL_BYTE_RR0 || byte == CONTROL_BYTE_RR1 || byte == CONTROL_BYTE_REJ0 || byte == CONTROL_BYTE_REJ1){
              *currentState = C_RCVD;
              *controlByte = byte;
            }
            else if(byte == FLAG){
                *currentState = FLAG_RCVD;
            }
            else{
                *currentState = START;
            }
            break;
        case C_RCVD:
            if(byte == (A_SET^(*controlByte))){
              *currentState = BCC2_RCVD;
            }        
            else if(byte == FLAG){
              *currentState = FLAG_RCVD;
            }    
            else{
              *currentState = START;
            }     
            break;
        case BCC2_RCVD:
            if(byte == FLAG){
              *currentState = END;
            }
            else{
              *currentState = START;
            }
            break;
        case END:
          	  break;
        default:
            break;
    }
}


int checkSucess(int fd, unsigned char* controlByte){
    unsigned char response[256];
    memset(response, 0, strlen((const char*) response));
	
	unsigned char byte;
    State state = START;
    while(state != END && data.alarmFlag == 0){
        if(read(fd,&byte,1) < 0){
          perror("Error reading byte");
			return 2;
        }
        responseStateMachine(&state,byte,controlByte);
    }

	if(*controlByte == CONTROL_BYTE_RR0 && data.ns == 1){
      return 0;
    }
    else if(*controlByte == CONTROL_BYTE_RR1 && data.ns == 0){
      return 0;
    }
    else if(*controlByte == CONTROL_BYTE_REJ0 && data.ns == 1){
      return 1;
    }
    else if(*controlByte == CONTROL_BYTE_REJ1 && data.ns == 0){
      return 1;
    }
    else
    {
      return 1;
    }

    return 1;
}

int llwrite(int fd, unsigned char* packet, int size){
    int frameSize;
    unsigned char controlByte;

    indexIteration++;

    do{
        if (data.numTries >= 1){
            printf("Retrying...\n");
        }
        frameSize = sendFrame(fd, packet, size);
        startAlarm();

        int val = checkSucess(fd, &controlByte);
        if ((val == 1 || val == 2) && !data.alarmFlag){
            data.alarmFlag = 1;
            alarm(0);
        }
    } while (data.numTries <= MAX_TRIES && data.alarmFlag);

    if (data.ns == 0) data.ns = 1;
    else if (data.ns == 1) data.ns = 0;

    stopAlarm();

    if (data.numTries > MAX_TRIES) {
        printf("max number of tries achieved\n");
        return -1;
    }
    data.numTries = 0;

    return frameSize;
}

void stateMachine(State *state, unsigned char byte){
    switch(*state){
        case START:
            if (byte == FLAG) *state = FLAG_RCVD;
            break;

        case FLAG_RCVD:
            if (byte == A_SET) *state = A_RCVD;
            else if (byte == FLAG) *state = FLAG_RCVD;
            else *state = START;
            break;
        
        case A_RCVD:
            if (byte == C_NS0 || byte == C_NS1){
                *state = C_RCVD;
            }
            else if (byte == FLAG) *state = FLAG_RCVD;
            else *state = START;
            break;

        case C_RCVD:
            if (byte == (A_SET ^ C_NS1) || byte == (A_SET ^ C_NS0)){
                *state = BCC1_RCVD;
            }
            else if (byte == FLAG) *state = FLAG_RCVD;
            else *state = START;
            break;

        case BCC1_RCVD:
            if (byte != FLAG) *state = DATA_RCVD;
            else if (byte == FLAG) *state = FLAG_RCVD;
            else *state = START;
            break;
        
        case DATA_RCVD:
            if (byte == FLAG){
                *state = END;
            }
            break;

        default:
            break;
    }
}

int readFrame(int fd, unsigned char* packet){
    int len = 0;
    unsigned char byte;
    State state = START;

    while(state != END){
    if(read(fd,&byte,1) < 0){
        perror("Error reading byte");
    }
    stateMachine(&state,byte);
    packet[len] = byte;
    len++;
    }

    return len;
}

int destuff(unsigned char* packet, unsigned char* destuffed, int size, unsigned char* dataPackets){
    for (int x = 0; x < 4; x++){
        destuffed[x] = packet[x];
    }

    int j = 4;
    int i;
    for (i = 4; i < size - 1; i++) {
        if (packet[i] == ESC && packet[i + 1] == (FLAG ^ STUFFING)){
            destuffed[j] = FLAG;
            i++;
            j++;
        }
        else if (packet[i] == ESC && packet[i + 1] == (ESC ^ STUFFING)){
            destuffed[j] = ESC;
            i++;
            j++;
        }
        else {
            destuffed[j] = packet[i];
            j++;
        }
    }
    destuffed[j++] = packet[i++];

    for (int x = 4; x < size - 2; x++){
        dataPackets[x-4] = destuffed[x];
    }


    return j;
}

int verifyPacket (unsigned char* destuffedFrame, int size, unsigned char* dataPackets){
    if (destuffedFrame[0] != FLAG || destuffedFrame[size - 1] != FLAG){
        printf("FLAG error\n");
        return 1;
    }
    else if (destuffedFrame[1] != A_SET){
        printf("A_SET error\n");
        return 1;
    }
    else if (destuffedFrame[2] != C_NS0 && destuffedFrame[2] != C_NS1){
        printf("C_NS0 error\n");
        return 1;
    }
    else if (destuffedFrame[3] != (A_SET ^ destuffedFrame[2])){
        printf("BCC1 error\n");
        return 1;
    }
    
    unsigned char bcc2 = 0x00;

    for (int i = 0; i < size - 6; i++)
        bcc2 ^= dataPackets[i];

    if (bcc2 != destuffedFrame[size - 2]){
        printf("BCC2 error\n");
        return 1;
    }

    return 0;
}

int buildResponse(unsigned char* response, char* flag){
    response[0] = FLAG;
    response[1] = A_SET;

    if (!strcmp("REJ1", flag)){
        response[2] = REJ1;
    }
    else if (!strcmp("REJ0", flag)){
        response[2] = REJ0;
    }
    else if (!strcmp("RR1", flag)){
        response[2] = RR1;
    }
    else if (!strcmp("RR0", flag)){
        response[2] = RR0;
    }

    response[3] = response[1] ^ response[2];
    response[4] = FLAG;

    return 0;
}

int llread(int fd, unsigned char* packet, unsigned char* dataPackets){
    unsigned char destuffedFrame[MAX_BUFFER_SIZE];
    unsigned char response[MAX_BUFFER_SIZE];
    memset(dataPackets, 0, strlen( (const char*) dataPackets));
    memset(response,0,strlen( (const char*) response));
    int size = 0, destuffedlen = 0;

    while(1){
        if ((size = readFrame(fd, packet)) > 0){
            destuffedlen = destuff(packet, destuffedFrame, size, dataPackets);

            if (verifyPacket(destuffedFrame, destuffedlen, dataPackets)){
                if (destuffedFrame[2] == C_NS0){
                    buildResponse(response, "REJ1");
                    write(fd, response, 5);
                }
                else if (destuffedFrame[2] == C_NS1){
                    buildResponse(response, "REJ0");
                    write(fd, response, 5);
                }

                return 0; 
            }
            else{
                if (destuffedFrame[2] == C_NS0){
                    buildResponse(response, "RR1");
                    write(fd, response, 5);
                }
                else if (destuffedFrame[2] == C_NS1){
                    buildResponse(response, "RR0");
                    write(fd, response, 5);
                }

                return destuffedlen;
            }

        }
    }
    return destuffedlen;
}

int setStruct(const char* serialPort, int status, char* baudrate){
    int fd = open(serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror(serialPort); 
        exit(-1); 
    }

    if (tcgetattr(fd, &oldtio) == -1) {
    perror("tcgetattr");
    exit(-1);
    }

    long br = strtol(baudrate,NULL,16);
    speed_t converted = checkBaudrate(br);

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = converted | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 5;
    newtio.c_cc[VMIN] = 10;

    cfsetspeed(&newtio, converted);

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
    }

    printf("New termios structure set\n\n");

    data.status = status;
    data.ns = 0;
    data.timeouts = MAX_TIMEOUTS;
    data.numTries = 0;
    data.alarmFlag = 1;

    return fd;
}

int llopen(const char* serialPort, int status, char* baudrate){
    int fd = setStruct(serialPort, status, baudrate);
    unsigned char buf[255];
    
    switch(status){
        case TRANSMITTER: //0

            buf[0] = FLAG;
            buf[1] = A_SET;
            buf[2] = C_SET;
            buf[3] = BCC_SET;
            buf[4] = FLAG;  

            do {
                if (data.numTries >= 1){
                    printf("Retrying...\n");
                }
                printf("Writing SET\n");
                write(fd, buf, 5);
                printf("SET sent.\n");
                startAlarm();
                data.alarmFlag = 1;

                printf("Waiting for UA...\n");
                int error = readUA(fd);

                if (!error){
                    stopAlarm();
                    data.alarmFlag = 0;
                    break;
                }
            } while (data.numTries <= MAX_TRIES && data.alarmFlag);

            stopAlarm();

            if (data.numTries > MAX_TRIES) {
                printf("max number of tries achieved\n");
                return -1;
            }
            data.numTries = 0;
            break;

        case RECEIVER: //1
            printf("Waiting for SET...\n");

            do {
                if (data.numTries >= 1){
                    printf("Didn't receive...\n");
                }
                startAlarm();
                data.alarmFlag = 1;

                int error = readSET(fd);
                if (!error){
                    stopAlarm();
                    data.alarmFlag = 0;
                    break;
                }
            } while (data.numTries <= MAX_TRIES && data.alarmFlag);

            stopAlarm();

            if (data.numTries >= MAX_TRIES) {
                printf("max number of tries achieved\n");
                return -1;
            }
            data.numTries = 0;

            buf[0] = FLAG;
            buf[1] = A_UA;
            buf[2] = C_UA;
            buf[3] = BCC_UA;
            buf[4] = FLAG;
            
            write(fd, buf, 5);  
            printf("UA Sent.\n");

            break;
        default:
            printf("Status error!\n");
            return -1;
    }
    
    return fd;
}

int llclose(int fd, int status){
    unsigned char buf[255];

    switch(status){
        case TRANSMITTER:
            buf[0] = FLAG;
            buf[1] = A_SET;
            buf[2] = C_DISC;
            buf[3] = BCC_DISC;
            buf[4] = FLAG;  

            do {
                write(fd, buf, 5);
                printf("DISC sent.\n");
                startAlarm();
                data.alarmFlag = 0;
                //reading DISC frame
                printf("Waiting for DISC...\n");
                int error = readDISC(fd);

                if (!error){
                    stopAlarm();
                    data.alarmFlag = 0;
                    break;
                }
                
            } while (data.numTries <= MAX_TRIES && data.alarmFlag);

            stopAlarm();

            if (data.numTries > MAX_TRIES) {
                printf("max number of tries achieved\n");
                return -1;
            }
            printf("Writing UA\n");
            buf[0] = FLAG;
            buf[1] = A_UA;
            buf[2] = C_UA;
            buf[3] = BCC_UA;
            buf[4] = FLAG;
            write(fd, buf, 5); 
            printf("UA Sent.\n"); 
            sleep(1);

            break;


        case RECEIVER:
            printf("Waiting for DISC...\n");

            do {
                if (data.numTries >= 1){
                    printf("Didn't receive...\n");
                }
                startAlarm();
                data.alarmFlag = 1;

                int error = readDISC(fd);
                if (!error){
                    stopAlarm();
                    data.alarmFlag = 0;
                    break;
                }
            } while (data.numTries <= MAX_TRIES && data.alarmFlag);

            stopAlarm();

            if (data.numTries > MAX_TRIES) {
                printf("max number of tries achieved\n");
                return -1;
            }

            buf[0] = FLAG;
            buf[1] = A_SET;
            buf[2] = C_DISC;
            buf[3] = BCC_DISC;
            buf[4] = FLAG;  
            write(fd, buf, 5);
            printf("DISC sent.\n");  
            
            // UA Handling
            printf("Waiting for UA...\n");

            if (readUA(fd)) return 1;

            break;
        default:
            printf("Status error!\n");
            return -1;
    }

    stopConnection(fd);
    return fd;
}
