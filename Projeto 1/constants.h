#define FLAG 0x7E
#define A_SET 0x03
#define C_SET 0x03
#define BCC1 (A_SET ^ C_SET)

#define A_UA 0x01
#define C_UA 0x07
#define BCC2 (A_UA ^ C_UA)
