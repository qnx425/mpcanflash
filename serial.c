#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "mpcanflash.h"

static HANDLE   hCom;
static char     Port[12] = { '\\', '\\', '.', '\\', 0, 0, 0, 0, 0, 0, 0, 0 };

static COMMTIMEOUTS timeouts = { 0, 0, 1000, 0, 500 };

LPCTSTR ErrorMessage(DWORD error) 
// Routine Description:
//      Retrieve the system error message for the last-error code
{ 

    LPVOID lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    return((LPCTSTR)lpMsgBuf);
}

int portOpen(char *port, DWORD br)
{
    strcat(Port, port);

    hCom = CreateFile(Port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hCom == INVALID_HANDLE_VALUE) 
    { 
        DWORD dwError = GetLastError();
        LPCTSTR errMsg = ErrorMessage(dwError);
        printf("Could not open port (%d): %s\n", (int)dwError, errMsg); 
        LocalFree((LPVOID)errMsg);
        return FALSE; 
    }

    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    
    if (FALSE == GetCommState(hCom, &dcb))
    { 
        DWORD dwError = GetLastError();
        LPCTSTR errMsg = ErrorMessage(dwError);
        printf("Could not get state (%d): %s\n", (int)dwError, errMsg); 
        LocalFree((LPVOID)errMsg);
        return FALSE; 
    }

    dcb.BaudRate = br;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.EvtChar  = 13;

    if (FALSE == SetCommState(hCom, &dcb))
    { 
        DWORD dwError = GetLastError();
        LPCTSTR errMsg = ErrorMessage(dwError);
        printf("Could not set state (%d): %s\n", (int)dwError, errMsg); 
        LocalFree((LPVOID)errMsg);
        return FALSE; 
    }

	if (FALSE == SetCommTimeouts(hCom, &timeouts))
    { 
        DWORD dwError = GetLastError();
        LPCTSTR errMsg = ErrorMessage(dwError);
        printf("Could not set timeout (%d): %s\n", (int)dwError, errMsg); 
        LocalFree((LPVOID)errMsg);
        return FALSE; 
    }
    
    if (FALSE == PurgeComm(hCom, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR))
    { 
        DWORD dwError = GetLastError();
        LPCTSTR errMsg = ErrorMessage(dwError);
        printf("Could not purge (%d): %s\n", (int)dwError, errMsg); 
        LocalFree((LPVOID)errMsg);
        return FALSE; 
    }

    return TRUE;
}

int portRead(char *rcv, DWORD NumberOfBytesToRead)
{
    DWORD NumberOfBytesRead;

    if (FALSE == ReadFile(hCom, rcv, NumberOfBytesToRead, &NumberOfBytesRead, NULL))
    { 
        DWORD dwError = GetLastError();
        LPCTSTR errMsg = ErrorMessage(dwError);
        printf("Could not read (%d): %s\n", (int)dwError, errMsg); 
        LocalFree((LPVOID)errMsg);
        return FALSE; 
    }
    if (NumberOfBytesToRead != NumberOfBytesRead) 
    {
        return FALSE; 
    }
    
    return TRUE;
}

int portWrite(char *snd, DWORD NumberOfBytesToWrite)
{
    DWORD NumberOfBytesWritten;
    
    if (FALSE == WriteFile(hCom, snd, NumberOfBytesToWrite, &NumberOfBytesWritten, NULL))
    { 
        DWORD dwError = GetLastError();
        LPCTSTR errMsg = ErrorMessage(dwError);
        printf("Could not write (%d): %s\n", (int)dwError, errMsg); 
        LocalFree((LPVOID)errMsg);
        return FALSE; 
    }
    
    if (NumberOfBytesToWrite != NumberOfBytesWritten) 
    {
        return FALSE; 
    }

    return TRUE;
}

int portClose(void)
{
    CloseHandle(hCom);
    
    return TRUE;
}

char cmd[8][12] = 
{
    "\r", 
    "N\r", 
    "Z0\r", 
    "S6\r", 
    "M8A600000\r", 
    "m00000000\r", 
    "O\r", 
    "C\r"
};

int protInit(void)
{
    char    rsp[16];
    DWORD   NumberOfBytesRead = 0;
    
    if (FALSE == portWrite(cmd[0], strlen(cmd[0]))) return ERR_USB_INIT1;
    Sleep(100);
    if (FALSE == portWrite(cmd[0], strlen(cmd[0]))) return ERR_USB_INIT1;
    Sleep(100);
    memset(rsp, 0, sizeof(rsp));
    ReadFile(hCom, rsp, sizeof(rsp), &NumberOfBytesRead, NULL);

    if (NumberOfBytesRead == 0)                     return ERR_USB_INIT1;
    if (rsp[0] == 0)                                return ERR_USB_INIT1;

    if (FALSE == portWrite(cmd[1], strlen(cmd[1]))) return ERR_USB_INIT1;
    if (FALSE == portRead (rsp, 6))                 return ERR_USB_INIT1;
    if (rsp[0] != 'N' || rsp[5] != 13)              return ERR_USB_INIT1;
    
    timeouts.ReadTotalTimeoutConstant = 5000;   // It is not error! - KVN
    
	if (FALSE == SetCommTimeouts(hCom, &timeouts))
    { 
        DWORD dwError = GetLastError();
        LPCTSTR errMsg = ErrorMessage(dwError);
        printf("Could not set timeout (%d): %s\n", (int)dwError, errMsg); 
        LocalFree((LPVOID)errMsg);
        return ERR_USB_INIT1; 
    }

    return ERR_NONE;
}

int protOpen(char *speed)
{
    char    rsp[16];
    int     i;

    cmd[3][1] = speed[0];

    for (i = 2; i < 7; i++)
    {
        if (FALSE == portWrite(cmd[i], strlen(cmd[i]))) return ERR_USB_INIT2;
        if (FALSE == portRead (rsp, 1))                 return ERR_USB_INIT2;
        
        if (rsp[0] != 13)                               return ERR_USB_INIT2;
    }
    
    return ERR_NONE;
}

int protClose(void)
{
    char    rsp[16];

    portWrite(cmd[7], strlen(cmd[7]));
    portRead (rsp, 1);
    
    return ERR_NONE;
}
