#include "ll.h"

int readResponseSET(int fd) {
    unsigned char received[255];
    int res = read(fd, received, 5);
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
    else if (received[3] != BCC2){
        printf("BCC2 error\n");
        return 1;
    }
    else{
        printf("SET is valid\n");
    }

    return 0;
}

int readResponseDISC(int fd) {
    unsigned char received[255];
    int res = read(fd, received, 5);
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

    if (tcsetattr(fd, TCSANOW, &data.oldtio) == -1) {
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
        frame[2] = BCC_NS0;
    else frame[2] = BCC_NS1;
    frame[3] = (A_SET ^ frame[2]);

    unsigned char bcc2 = 0x00;

	for (int i = 0; i < size; i++)
		bcc2 ^= packet[i];

    printf("%d\n", bcc2);

    for (int x = 0; x < size; x++){
        if (packet[x] == FLAG || packet[x] == ESC){
            frame[index + 4] = ESC;
            frame[index + 5] = packet[x] ^ STUFFING;
            index++;
        }
        else frame[index + 4] = packet[x];

        index++;
    }

    // Control back bytes
    if (bcc2 == FLAG || bcc2 == ESC){
        frame[4+index] = ESC;
        frame[4+index+1] = bcc2 ^ STUFFING;
        index++;
    }
    else frame[4 + index] = bcc2;
    index++;
    frame[4 + index] = FLAG;
    

    int totalSize = index + 5;

    write(fd, frame, totalSize);
    printf("Size sent: %d\n", totalSize);

    return totalSize;  
}

int checkSucess(int fd, unsigned char* packet){
    unsigned char response[256];
    read(fd, response, 5);

    if (response[0] != FLAG || response[4] != FLAG){
        printf("FLAG error\n");
        return 1;
    }
    else if (response[1] != A_SET){
        printf("A_SET error\n");
        return 1;
    }
    else if (response[3] != BCC1){
        printf("BCC1 error\n");
        return 1;
    }
    
    switch(response[2]){
        case 0x01:
            return 1;
        case 0x81:
            return 1;
        case 0x05:
            return 0;
        case 0x85:
            return 0;
        default:
            return 1;
    }

    return 1;
}

int llwrite(int fd, unsigned char* packet, int size){
    int frameSize, res;

    do{
        if (data.numTries >= 1){
            printf("Retrying...\n");
        }
        frameSize = sendFrame(fd, packet, size);
        printf("Packet sent.\n");
        startAlarm();
        data.alarmFlag = 1;

        int val = checkSucess(fd, packet);
        if (!val){
            if (data.ns == 0) data.ns = 1;
            else if (data.ns == 1) data.ns = 0;
            stopAlarm();
            data.alarmFlag = 0;
            break;
        }
        data.numTries++;
    } while (data.numTries < MAX_TRIES && data.alarmFlag);

    stopAlarm();

    if (data.numTries >= MAX_TRIES) {
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
            break;
        
        case A_RCVD:
            if (byte == BCC_NS0 || byte == BCC_NS1){
                *state = C_RCVD;
            }
            break;

        case C_RCVD:
            if (byte == (A_SET ^ BCC_NS1) || byte == (A_SET ^ BCC_NS0)){
                *state = BCC1_RCVD;
            }
            break;

        case BCC1_RCVD:
            if (byte != FLAG) *state = DATA_RCVD;
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
    int len = 0, msgIndex = 0;
    unsigned char byte, bcc2 = 0x00;
    State state = START; 
    unsigned char msg[3000];
    while (1){
        read(fd, &byte, 1);
        stateMachine(&state, byte);
        packet[len] = byte;
        len++;
        if (state == END){
            break;
        }
    }

    return len;
}

int destuff(unsigned char* packet, unsigned char* destuffed, int size, unsigned char* message){
    for (int x = 0; x < 4; x++){
        destuffed[x] = packet[x];
    }

    int j = 4;
	int i;
	for (i = 4; i < size - 1; i++) {
		if (packet[i] == ESC && packet[i+1] == (FLAG ^ STUFFING)){
            destuffed[j] = FLAG;
            i++;
            j++;
        }
        else if (packet[i] == ESC && packet[i+1] == (ESC ^ STUFFING)){
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
        message[x-4] = destuffed[x];
    }

    printf("\n");

    printf("Tamanho da frame: %d\n", i);

    return j;
}

int verifyPacket (unsigned char* destuffedFrame, int size, unsigned char* message){
    if (destuffedFrame[0] != FLAG || destuffedFrame[size - 1] != FLAG){
        printf("FLAG error\n");
        return 1;
    }
    else if (destuffedFrame[1] != A_SET){
        printf("A_SET error\n");
        return 1;
    }
    else if (destuffedFrame[2] != BCC_NS0 && destuffedFrame[2] != BCC_NS1){
        printf("BCC_NS0 error\n");
        return 1;
    }
    else if (destuffedFrame[3] != (A_SET ^ BCC_NS1) && destuffedFrame[3] != (A_SET ^ BCC_NS0)){
        printf("BCC1 error\n");
        return 1;
    }
    
    unsigned char bcc2 = 0x00;

	for (int i = 0; i < size - 6; i++)
		bcc2 ^= message[i];

    if (bcc2 != destuffedFrame[size - 2]){
        printf("BCC2 error\n");
        return 1;
    }

    return 0;
}

int buildResponse(unsigned char* response, unsigned char* flag){
    response[0] = FLAG;
    response[1] = A_SET;
    response[3] = BCC1;
    response[4] = FLAG;
    

    if (!strcmp("REJ1", flag)){
        response[2] = 0x01;
    }
    else if (!strcmp("REJ0", flag)){
        response[2] = 0x81;
    }
    else if (!strcmp("RR1", flag)){
        response[2] = 0x85;
    }
    else if (!strcmp("RR0", flag)){
        response[2] = 0x05;
    }

    return 0;
}

int llread(int fd, unsigned char* packet, unsigned char* message){
    unsigned char destuffedFrame[3000];
    unsigned char response[3000];
    memset(message,0,strlen(message));
    memset(response,0,strlen(response));
    unsigned char* answer;
    int size = 0, destuffedlen = 0;

    while(1){
        if ((size = readFrame(fd, packet)) > 0){
            destuffedlen = destuff(packet, destuffedFrame, size, message);

            printf("Data size: %d\n", size);

            if (verifyPacket(destuffedFrame, destuffedlen, message)){
                if (destuffedFrame[2] == BCC_NS0){
                    buildResponse(response, "REJ1");
                    write(fd, response, 5);
                    printf("REJ sent: 1\n");
                }
                else if (destuffedFrame[2] == BCC_NS1){
                    buildResponse(response, "REJ0");
                    write(fd, response, 5);
                    printf("REJ sent: 0\n");
                }

                return 0; 
            }
            else{
                if (destuffedFrame[2] == BCC_NS0){
                    buildResponse(response, "RR1");
                    write(fd, response, 5);
                    printf("RR sent: 1\n");
                }
                else if (destuffedFrame[2] == BCC_NS1){
                    buildResponse(response, "RR0");
                    write(fd, response, 5);
                    printf("RR sent: 0\n");
                }

                return destuffedlen;
            }

        }
    }
    return destuffedlen;
}

int setStruct(const char* serialPort, int status){
    int fd = open(serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror(serialPort); 
        exit(-1); 
    }

    if (tcgetattr(fd, &data.oldtio) == -1) {
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&data.newtio, sizeof(data.newtio));
    data.newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    data.newtio.c_iflag = IGNPAR;
    data.newtio.c_oflag = 0;

    data.newtio.c_lflag = 0;

    data.newtio.c_cc[VTIME] = 0;
    data.newtio.c_cc[VMIN] = 5;

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &data.newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    strcpy(data.serialPort, serialPort);
    data.status = status;
    data.ns = 0;
    data.timeouts = MAX_TIMEOUTS;
    data.numTries = 0;
    data.alarmFlag = 1;

    return fd;
}

int llopen(const char* serialPort, int status){
    int fd = setStruct(serialPort, status);
    int res;
    unsigned char buf[255], received[255];
    
    switch(status){
        case TRANSMITTER: //TRANSMITTER

            buf[0] = FLAG;
            buf[1] = A_SET;
            buf[2] = C_SET;
            buf[3] = BCC1;
            buf[4] = FLAG;  

            do {
                if (data.numTries >= 1){
                    printf("Retrying...\n");
                }
                printf("Writing SET\n");
                res = write(fd, buf, 5);
                printf("SET sent.\n");
                startAlarm();
                data.alarmFlag = 1;

                printf("Waiting for UA...\n");
                int error = readResponseSET(fd);

                if (!error){
                    stopAlarm();
                    data.alarmFlag = 0;
                    break;
                }
            } while (data.numTries < MAX_TRIES && data.alarmFlag);

            stopAlarm();

            if (data.numTries >= MAX_TRIES) {
                printf("max number of tries achieved\n");
                return -1;
            }
            data.numTries = 0;
            break;


        case RECEIVER: //RECEIVER
            printf("Waiting for SET...\n");
            res = read(fd, received, 5);
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
            res = write(fd, buf, 5);  
            printf("UA Sent.\n");

            break;
        default:
            printf("Status error!\n");
            return -1;
    }
    
    return fd;
}

int llclose(int fd, int status){
    unsigned char buf[255], received[255];
    int res;

    switch(status){
        case TRANSMITTER:
            buf[0] = FLAG;
            buf[1] = A_SET;
            buf[2] = C_DISC;
            buf[3] = BCC_DISC;
            buf[4] = FLAG;  

            do {
                res = write(fd, buf, 5);
                printf("DISC sent.\n");
                startAlarm();
                data.alarmFlag = 0;
                //reading DISC frame
                printf("Waiting for DISC...\n");
                int error = readResponseDISC(fd);

                if (!error){
                    stopAlarm();
                    data.alarmFlag = 0;
                    break;
                }
                
            } while (data.numTries < MAX_TRIES && data.alarmFlag);

            stopAlarm();

            if (data.numTries >= MAX_TRIES) {
                printf("max number of tries achieved\n");
                return -1;
            }
            else {
                printf("Writing UA\n");
                buf[0] = FLAG;
                buf[1] = A_UA;
                buf[2] = C_UA;
                buf[3] = BCC2;
                buf[4] = FLAG;
                res = write(fd, buf, 5); 
                printf("UA Sent.\n"); 
                sleep(1);
            }
            break;


        case RECEIVER:
            printf("Waiting for DISC...\n");
            res = read(fd, received, 5);
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
            res = write(fd, buf, 5);
            printf("DISC sent.\n");  
            
            // UA Handling
            printf("Waiting for UA...\n");

            res = read(fd, received, 5);
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
            else {
                printf("UA is valid\n");
            }
            break;
        default:
            printf("Status error!\n");
            return -1;
    }

    stopConnection(fd);
    return fd;
}
