#ifndef DEVICEDATA_H
#define DEVICEDATA_H

// Types of memory regions
#define PROGRAM_MEMORY      0x01
#define EEPROM_MEMORY       0x02
#define CONFIG_MEMORY       0x03
#define USERID_MEMORY       0x04
#define END_OF_TYPES_LIST   0xFF

// Added by KVN
#define APP_RESET_VECTOR    0x0800

#define PIC18FXXK80_EEPROM  1024

#define PIC18FX5K80_SIZE    32768
#define PIC18FX6K80_SIZE    65536

#define PIC18F66K80         (0x60E0 >> 5) // (64k)
#define PIC18F46K80         (0x6100 >> 5) // (64k)
#define PIC18F26K80         (0x6120 >> 5) // (64k)
#define PIC18F65K80         (0x6140 >> 5) // (32k)
#define PIC18F45K80         (0x6160 >> 5) // (32k)
#define PIC18F25K80         (0x6180 >> 5) // (32k)
#define PIC18LF66K80        (0x61C0 >> 5) // (64k)
#define PIC18LF46K80        (0x61E0 >> 5) // (64k)
#define PIC18LF26K80        (0x6200 >> 5) // (64k)
#define PIC18LF65K80        (0x6220 >> 5) // (32k)
#define PIC18LF45K80        (0x6240 >> 5) // (32k)
#define PIC18LF25K80        (0x6260 >> 5) // (32k)

extern  int DeviceData(unsigned char *);

#endif // DEVICEDATA_H
