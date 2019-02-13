#include <stdio.h>
#include <string.h>
#include "DeviceData.h"
#include "mpcanflash.h"

int DeviceData(unsigned char *d) {
    unsigned short  dev;
    sQuery         *ptr;
    
    ptr   = (sQuery *)d;
    dev   = *(unsigned short *)(d + 1);
    dev >>= 5;
    
    if (dev < PIC18F66K80 || dev > PIC18LF25K80)
        return ERR_DEVICE_NOT_FOUND;
    
    memset(d, 0xFF, sizeof(sQuery));
    
    ptr->PacketDataFieldSize    = 8;
    ptr->DeviceFamily           = DEVICE_FAMILY_PIC18;

    ptr->mem[0].Type            = TypeProgramMemory;
    ptr->mem[0].Address         = APP_RESET_VECTOR;
    
    switch (dev) {
        case PIC18F66K80:
             ptr->mem[0].Length = PIC18FX6K80_SIZE - APP_RESET_VECTOR;
             DEBUGMSG("Device: PIC18F66K80");
             break;
             
        case PIC18F46K80:
             ptr->mem[0].Length = PIC18FX6K80_SIZE - APP_RESET_VECTOR;
             DEBUGMSG("Device: PIC18F46K80");
             break;
             
        case PIC18F26K80:
             ptr->mem[0].Length = PIC18FX6K80_SIZE - APP_RESET_VECTOR;
             DEBUGMSG("Device: PIC18F26K80");
             break;
             
        case PIC18LF66K80:
             ptr->mem[0].Length = PIC18FX6K80_SIZE - APP_RESET_VECTOR;
             DEBUGMSG("Device: PIC18LF66K80");
             break;
             
        case PIC18LF46K80:
             ptr->mem[0].Length = PIC18FX6K80_SIZE - APP_RESET_VECTOR;
             DEBUGMSG("Device: PIC18LF46K80");
             break;
             
        case PIC18LF26K80:
             ptr->mem[0].Length = PIC18FX6K80_SIZE - APP_RESET_VECTOR;
             DEBUGMSG("Device: PIC18LF26K80");
             break;
             
        case PIC18F65K80:
             ptr->mem[0].Length = PIC18FX5K80_SIZE - APP_RESET_VECTOR;
             DEBUGMSG("Device: PIC18F65K80");
             break;
             
        case PIC18F45K80:
             ptr->mem[0].Length = PIC18FX5K80_SIZE - APP_RESET_VECTOR;
             DEBUGMSG("Device: PIC18F45K80");
             break;
             
        case PIC18F25K80:
             ptr->mem[0].Length = PIC18FX5K80_SIZE - APP_RESET_VECTOR;
             DEBUGMSG("Device: PIC18F25K80");
             break;
             
        case PIC18LF65K80:
             ptr->mem[0].Length = PIC18FX5K80_SIZE - APP_RESET_VECTOR;
             DEBUGMSG("Device: PIC18LF65K80");
             break;
             
        case PIC18LF45K80:
             ptr->mem[0].Length = PIC18FX5K80_SIZE - APP_RESET_VECTOR;
             DEBUGMSG("Device: PIC18LF45K80");
             break;
             
        case PIC18LF25K80:
             ptr->mem[0].Length = PIC18FX5K80_SIZE - APP_RESET_VECTOR;
             DEBUGMSG("Device: PIC18LF25K80");
             break;
        
        default:
             (void)printf("Unknown device\nDevice ID: 0x%X\n", dev);
             return ERR_DEVICE_NOT_FOUND;
    }
    
    ptr->mem[1].Type            = TypeEEPROM;
    ptr->mem[1].Address         = 0xF00000;
    ptr->mem[1].Length          = PIC18FXXK80_EEPROM;
    
    ptr->mem[2].Type            = TypeConfigWords;
    ptr->mem[2].Address         = 0x300000;
    ptr->mem[2].Length          = 14;

    return ERR_NONE;
}
