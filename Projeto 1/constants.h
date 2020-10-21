#define FLAG 0x7E
#define A_SET 0x03
#define C_SET 0x03
#define C_DISC 0x0B
#define BCC1 (A_SET ^ C_SET)

#define A_UA 0x01
#define C_UA 0x07
#define BCC2 (A_UA ^ C_UA)

#define BCC_DISC (A_SET ^ C_DISC)

#define BCC_NS0     0x00
#define BCC_NS1     0x40

#define ESC 0x7D
#define STUFFING 0x20
