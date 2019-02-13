/****************************************************************************
 File        : usb-windows.c
 Description : Encapsulates all nonportable, Linux-specific USB I/O code
               within the mphidflash program.  Each supported operating
               system has its own source file, providing a common calling
               syntax to the portable sections of the code.

 History     : 2009-12-26  Thomas Fischl, Dominik Fisch (www.FundF.net)
                 * Initial windows support
               
 License     : Copyright (C) 2009 Thomas Fischl, Dominik Fisch (www.FundF.net)

               This file is part of 'mphidflash' program.

               'mphidflash' is free software: you can redistribute it and/or
               modify it under the terms of the GNU General Public License
               as published by the Free Software Foundation, either version
               3 of the License, or (at your option) any later version.

               'mphidflash' is distributed in the hope that it will be useful,
               but WITHOUT ANY WARRANTY; without even the implied warranty
               of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
               See the GNU General Public License for more details.

               You should have received a copy of the GNU General Public
               License along with 'mphidflash' source code.  If not,
               see <http://www.gnu.org/licenses/>.

 ****************************************************************************/

#include <stdio.h>
#include <windows.h>
#include <setupapi.h>
#include "mpcanflash.h"
#include "DeviceData.h"
#include "serial.h"

unsigned char        usbBufX[65];
unsigned char *      usbBuf = &usbBufX[1];

static int  currState = 0;
static char cmdStr[24]   = { 't', '4', '5', '0', '8', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0 };

/**
 * Parse hex value of given string
 *
 * @param line Input string
 * @param len Count of characters to interpret
 * @param value Pointer to variable for the resulting decoded value
 * @return 0 on error, 1 on success
 */
unsigned char parseHex(char * line, unsigned char len, unsigned long * value) {
    *value = 0;
    while (len--) {
        if (*line == 0) return 0;
        *value <<= 4;
        if ((*line >= '0') && (*line <= '9')) {
           *value += *line - '0';
        } else if ((*line >= 'A') && (*line <= 'F')) {
           *value += *line - 'A' + 10;
        } else if ((*line >= 'a') && (*line <= 'f')) {
           *value += *line - 'a' + 10;
        } else return 0;
        line++;
    }
    return 1;
}

ErrorCode usbWrite(const char len, const char read)
{
	int  i, cmd;
    char rspStr[24];
    unsigned long tmp;
    
    cmd = usbBuf[0];

    if (cmd == PROGRAM_DEVICE)
    {
        if (currState != PROGRAM_DEVICE)
        {
            cmdStr[3] = '0';
            cmdStr[5] = '0';
            cmdStr[6] = cmd + 0x30;
            for (i = 1; i < 8; i++) sprintf(&cmdStr[5+2*i], "%02x", ((unsigned char *)usbBuf)[i]);
            cmdStr[21] = 13;
#ifdef DEBUG
            printf("cmdStr: %s\n", cmdStr);

            if (portWrite(cmdStr, 22)) {
                if (portRead(rspStr, 2)) {
                    if (rspStr[0] == 'z' && rspStr[1] == 13) {
                        if (FALSE == portRead(rspStr, 22)) {
                            DEBUGMSG("portRead(1) slow error");
                            return ERR_USB_READ;
                        }
                    }
                    else
                    {
                        DEBUGMSG("portRead(1) response error");
                        return ERR_USB_READ;
                    }
                }
                else
                {
                    DEBUGMSG("portRead(1) fast error");
                    return ERR_USB_READ;
                }
            }
            else
            {
                DEBUGMSG("portWrite(1) error");
                return ERR_USB_WRITE;
            }
#else
            if (FALSE == portWrite(cmdStr, 22))         return ERR_USB_WRITE;
            if (FALSE == portRead(rspStr, 2))           return ERR_USB_READ;
            if (rspStr[0] != 'z' || rspStr[1] != 13)    return ERR_USB_READ;
            if (FALSE == portRead(rspStr, 22))          return ERR_USB_READ;
#endif    
        }
        cmdStr[3] = '1';
        for (i = 0; i < 8; i++) sprintf(&cmdStr[5+2*i], "%02x", ((unsigned char *)usbBuf)[i+8]);
    }
    else
    {
        cmdStr[3] = '0';
        cmdStr[5] = '0';
        cmdStr[6] = cmd + 0x30;
        for (i = 1; i < 8; i++) sprintf(&cmdStr[5+2*i], "%02x", ((unsigned char *)usbBuf)[i]);
    }

    currState = cmd;

    cmdStr[21] = 13;
#ifdef DEBUG
    printf("cmdStr: %s\n", cmdStr);

    if (portWrite(cmdStr, 22)) {
        if (portRead(rspStr, 2)) {
            if (rspStr[0] == 'z' && rspStr[1] == 13) {
                if (FALSE == portRead(rspStr, 22)) {
                    DEBUGMSG("portRead(2) slow error");
                    return ERR_USB_READ;
                }
            }
            else
            {
                DEBUGMSG("portRead(2) response error");
                return ERR_USB_READ;
            }
        }
        else
        {
            DEBUGMSG("portRead(2) fast error");
            return ERR_USB_READ;
        }
    }
    else
    {
        DEBUGMSG("portWrite(2) error");
        return ERR_USB_WRITE;
    }
#else
    if (FALSE == portWrite(cmdStr, 22))         return ERR_USB_WRITE;
    if (FALSE == portRead(rspStr, 2))           return ERR_USB_READ;
    if (rspStr[0] != 'z' || rspStr[1] != 13)    return ERR_USB_READ;
    if (FALSE == portRead(rspStr, 22))          return ERR_USB_READ;
#endif    

    if (read) 
    {
        switch (cmd)
        {
            case QUERY_DEVICE:
                 if (parseHex(&rspStr[7], 4, &tmp))
                 {
                     bufWrite32(usbBuf, 1, tmp);
                     return (DeviceData(usbBuf));
                 }
                 break;

            case GET_DATA:
                 for (i = 0; i < 8; i++)
                    if (parseHex(&rspStr[5+2*i], 2, &tmp))
                        bufWrite32(usbBuf, (8+i), tmp);
                 break;
            
            default:
                 break;
        }
    }

	return ERR_NONE;
}
