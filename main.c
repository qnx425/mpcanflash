/****************************************************************************
 File        : main.c
 Description : Main source file for 'mphidflash,' a simple command-line tool for
               communicating with Microchips USB HID-Bootloader and downloading new
               firmware. 

 History     : 2009-02-19  Phillip Burgess
                 * Initial implementation
               2009-12-26  Thomas Fischl, Dominik Fisch (www.FundF.net)
                 * Renamed 'ubw32' to 'mphidflash'
               2010-12-28  Petr Olivka
                 * program and verify only data for defined memory areas
                 * send only even length of data to PIC
               
 License     : Copyright (C) 2009 Phillip Burgess
               Copyright (C) 2009 Thomas Fischl, Dominik Fisch (www.FundF.net)
               Copyright (C) 2010 Petr Olivka

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
#include <string.h>
#include <windows.h>
#include "mpcanflash.h"
#include "serial.h"

sQuery devQuery;

extern unsigned char * usbBuf;  /* In usb code */

/* Program's actions aren't necessarily performed in command-line order.
   Bit flags keep track of options set or cleared during input parsing,
   then are singularly checked as actions are performed.  Some actions
   (such as writing) don't have corresponding bits here; certain non-NULL
   string values indicate such actions should occur. */
#define ACTION_UNLOCK (1 << 0)
#define ACTION_ERASE  (1 << 1)
#define ACTION_VERIFY (1 << 2)
#define ACTION_RESET  (1 << 3)
#define ACTION_SIGN   (1 << 4)

/****************************************************************************
 Function    : main
 Description : mphidflash program startup; parse command-line input and issue
               commands as needed; return program status.
 Returns     : int  0 on success, else various numeric error codes.
 ****************************************************************************/
int main(
  int   argc,
  char *argv[])
{
    // KVN
	char        *comPort   = "COM1";
	unsigned int baudRate  = 115200;
	char        *canSpeed  = "6";
	unsigned int deviceID  = -1;
    //
	char        *hexFile   = NULL,
	             actions   = ACTION_VERIFY,
	             eol;        /* 1 = last command-line arg */
	ErrorCode    status    = ERR_NONE;
	int          i;
//	unsigned int vendorID  = 0x04d8,
//	             productID = 0x003c;

	const char * const errorString[ERR_EOL] = {
		"Missing or malformed command-line argument",
		"Command not recognized",
		"Device not found (is device attached and in Bootloader mode?)",
		"SLCAN initialization failed (phase 1)",
		"SLCAN initialization failed (phase 2)",
		"COM port could not be opened",
		"COM port write error",
		"COM port read error",
		"Could not open hex file for input",
		"Could not query hex file size",
		"Could not map hex file to memory",
		"Unrecognized or invalid hex file syntax",
		"Bad end-of-line checksum in hex file",
		"Unsupported record type in hex file",
		"Verify failed"
	};

	/* To create a sensible sequence of operations, all command-line
	   input is processed prior to taking any actions.  The sequence
	   of actions performed may not always directly correspond to the
	   order or quantity of input; commands follow precedence, not
	   input order.  For example, the action corresponding to the "-u"
	   (unlock) command must take place early on, before any erase or
	   write operations, even if specified late on the command line;
	   conversely, "-r" (reset) should always be performed last
	   regardless of input order.  In the case of duplicitous
	   commands (e.g. if multiple "-w" (write) commands are present),
	   only the last one will take effect.

	   The precedence of commands (first to last) is:

	   -v and -p <hex>  USB vendor and/or product IDs
	   -u               Unlock configuration memory
	   -e               Erase program memory
	   -n               No verify after write
	   -w <file>        Write program memory
	   -s               Sign code
	   -r               Reset */

	for(i=1;(i < argc) && (ERR_NONE == status);i++) {
		eol = (i >= (argc - 1));
		if(!strncmp(argv[i],"-d",2)) {
			if(eol || (1 != sscanf(argv[++i],"%x",&deviceID)))
				status = ERR_CMD_ARG;
		} else if(!strncmp(argv[i],"-b",2)) {
			if(eol || (1 != sscanf(argv[++i],"%d",&baudRate)))
				status = ERR_CMD_ARG;
		} else if(!strncmp(argv[i],"-p",2)) {
			if(eol) {
				status   = ERR_CMD_ARG;
			} else {
				comPort  = argv[++i];
			}
		} else if(!strncmp(argv[i],"-S",2)) {
			if(eol) {
				status   = ERR_CMD_ARG;
			} else {
				canSpeed = argv[++i];
			}
		} else if(!strncmp(argv[i],"-e",2)) {
			actions |= ACTION_ERASE;
		} else if(!strncmp(argv[i],"-n",2)) {
			actions &= ~ACTION_VERIFY;
		} else if(!strncmp(argv[i],"-w",2)) {
			if(eol) {
				status   = ERR_CMD_ARG;
			} else {
				hexFile  = argv[++i];
				actions |= ACTION_ERASE;
			}
		} else if(!strncmp(argv[i],"-s",2)) {
			actions |= ACTION_SIGN;
		} else if(!strncmp(argv[i],"-r",2)) {
			actions |= ACTION_RESET;
		} else if(!strncmp(argv[i],"-h",2) ||
		          !strncmp(argv[i],"-?",2)) {
                  (void)printf(
"mpcanflash v%d.%d: a Microchip CAN Bootloader utility\n\n"
"Option     Description                                      Default\n"
"-------------------------------------------------------------------------\n"
"-d <id>    Device ID in hex format (mandatory)              No\n"
"-p <port>  COM port                                         COM1\n"
"-b <baud>  Baud rate                                        115200\n"
"-S <speed> CAN bus speed as described in LAWICEL protocol   6\n"
"-w <file>  Write hex file to device (with erasing first)    None\n"
"-e         Erase device code space                          No erase\n"
"-s         Sign flash                                       No sign\n"
"-r         Reset device on program exit                     No reset\n"
"-n         No verify after write                            Verify on\n"
"-h or -?   Help\n", VERSION_MAIN, VERSION_SUB);
			return 0;
		} else {
			status = ERR_CMD_UNKNOWN;
		}
	}
    //////////////////////////// KVN ///////////////////////
	if(ERR_NONE != status) {
		(void)printf("%s Error",argv[0]);
		if(status <= ERR_EOL)
			(void)printf(": %s\n",errorString[status - 1]);
		else
			(void)puts(" of indeterminate type.");
        
        return (int)status;
	}
    
    if (deviceID == -1) {
        (void)printf("%s error: invalid Device ID", argv[0]);
        return (int)status;
    }
    #ifdef DEBUG
    printf("Device ID: %x\n", deviceID);
    #endif
    
    if (portOpen(comPort, baudRate))
    {
        if (ERR_NONE == (status = protInit()))
        {
            if (ERR_NONE == (status = protOpen(canSpeed)))
            {
                usbBuf[0] = BOOT_MODE;
                bufWrite32(usbBuf, 1, deviceID);
                status    = usbWrite(5,0);

				if (ERR_NONE == status) {
					usbBuf[0] = QUERY_DEVICE;
					if(ERR_NONE == (status = usbWrite(1,1))) {
						memcpy( &devQuery, usbBuf, 64 );
						i = 0;
						while ( devQuery.mem[ i ].Type != TypeEndOfTypeList ) i++;
						devQuery.memBlocks = i;
						for ( i = 0; i < devQuery.memBlocks; i++ ) {
						  devQuery.mem[i].Address = convertEndian(devQuery.mem[i].Address);
						  devQuery.mem[i].Length = convertEndian(devQuery.mem[i].Length);
						  if(devQuery.mem[i].Type == TypeProgramMemory) {
						    //(void)printf(": %d bytes free\n",devQuery.mem[i].Length);
						    }
						}

						//(void)printf("Device family: ");
 	 	 	 			switch (devQuery.DeviceFamily)
							{
							case DEVICE_FAMILY_PIC18:
								hexSetBytesPerAddress(1);
								//(void)printf("PIC18\n");
								break;
							case DEVICE_FAMILY_PIC24:
								hexSetBytesPerAddress(2);
								//(void)printf("PIC24\n");
								break;
							case DEVICE_FAMILY_PIC32:
								hexSetBytesPerAddress(1);
								//(void)printf("PIC32\n");
								break;
							default:
								hexSetBytesPerAddress(1);
								//(void)printf("Unknown. Bytes per address set to 1.\n");
								break;
						}


					}
					(void)putchar('\n');
                    /*
					if((ERR_NONE == status) && (actions & ACTION_UNLOCK)) {
						(void)puts("Unlocking configuration memory...");
						usbBuf[0] = UNLOCK_CONFIG;
						usbBuf[1] = UNLOCKCONFIG;
						status    = usbWrite(2,0);
					}
					// disable all configuration blocks in devQuery if locked
					if ( !( actions & ACTION_UNLOCK ) ) {
						for ( i = 0; i < devQuery.memBlocks; i++ )
						    if ( devQuery.mem[ i ].Type == TypeConfigWords )
							devQuery.mem[ i ].Type = 0;
					}
                    */
					/* Although the next actual operation is ACTION_ERASE,
					   if we anticipate hex-writing in a subsequent step,
                               attempt opening file now so we can display any error
					   message quickly rather than waiting through the whole
					   erase operation (it's usually a simple filename typo). */
					if((ERR_NONE == status) && hexFile &&
					   (ERR_NONE != (status = hexOpen(hexFile))))
						hexFile = NULL;  /* Open or mmap error */

					if((ERR_NONE == status) && (actions & ACTION_ERASE)) {
						(void)puts("Erasing...");
						usbBuf[0] = ERASE_DEVICE;
						status    = usbWrite(1,0);
						/* The query here isn't needed for any technical
						   reason, just makes the presentation better.
						   The ERASE_DEVICE command above returns
						   immediately...subsequent commands can be made
						   but will pause until the erase cycle completes.
						   So this query just keeps the "Writing" message
						   or others from being displayed prematurely. */
						//usbBuf[0] = QUERY_DEVICE;
						//status    = usbWrite(1,1);
					}

					if(hexFile) {
						if(ERR_NONE == status) {
						  (void)printf("Writing hex file '%s'\n",hexFile);
						  status = hexWrite((actions & ACTION_VERIFY) != 0);
						  (void)putchar('\n');
						}
						hexClose();
					}

					if((ERR_NONE == status) && (actions & ACTION_SIGN)) {
						(void)puts("Signing flash...");
						usbBuf[0] = SIGN_FLASH;
						status = usbWrite(1,0);
					}

					if((ERR_NONE == status) && (actions & ACTION_RESET)) {
						(void)puts("Resetting device...");
						usbBuf[0] = RESET_DEVICE;
						status = usbWrite(1,0);
					}
				}
                else {
                    (void)printf("No response from device 0x%x\n", deviceID);
                }
                protClose();
                portClose();
            }
        }
    }
    else status   = ERR_USB_OPEN;
    //////////////////////////// KVN ///////////////////////

	if(ERR_NONE != status) {
		(void)printf("%s Error",argv[0]);
		if(status <= ERR_EOL)
			(void)printf(": %s\n",errorString[status - 1]);
		else
			(void)puts(" of indeterminate type.");
	}

	return (int)status;
}
