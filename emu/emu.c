#include <stdio.h>
#include <windows.h>

int MaintReq(char *);

static HANDLE hCom;

const  char   CR     = 13;
const  char   BELL   = 7;
const  char   RSP    = 'z';

const  char  *Serial = "N1976\r";

static char   Port[12];

int main(int argc, char *argv[]) {
//    BOOL fSuccess;
    DWORD dwEvtMask;
    DWORD dwBytesRead = 0;
    char buf[32];
    
    if (argc != 2)
    {
        puts("COM port must be specified.");
        return 1;
    }
    
    strcpy(Port, "\\\\.\\");
    strcat(Port, argv[1]);
    
	hCom = CreateFile(Port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hCom == INVALID_HANDLE_VALUE) {
        printf("CreateFile failed with error %d.\n", (int)GetLastError());
        return 1;
    }
    
    COMMTIMEOUTS timeouts={ MAXDWORD, 0, 0, 0, 0 };
    
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    
    if (!GetCommState(hCom, &dcb)) {
      printf ("GetCommState failed with error %d.\n", (int)GetLastError());
      return (2);
    }

    dcb.BaudRate = CBR_115200;     //  baud rate
    dcb.ByteSize = 8;             //  data size, xmit and rcv
    dcb.Parity   = NOPARITY;      //  parity bit
    dcb.StopBits = ONESTOPBIT;    //  stop bit
    dcb.EvtChar  = 13;

    if (!SetCommState(hCom, &dcb)) {
      printf ("SetCommState failed with error %d.\n", (int)GetLastError());
      return (3);
    }

	if(!SetCommTimeouts(hCom, &timeouts)){
        printf("SetCommTimeouts failed with error %d.\n", (int)GetLastError());
        return 4;
	}
    
    if (!SetCommMask(hCom, EV_RXFLAG))  {
        printf("SetCommMask failed with error %d.\n", (int)GetLastError());
        return 5;
    }

    if (!PurgeComm(hCom, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR)) {
        printf("PurgeComm failed with error %d.\n", (int)GetLastError());
        return 6;
    }
    while (1) {
        if (WaitCommEvent(hCom, &dwEvtMask, NULL)) {
            if (dwEvtMask & EV_RXFLAG) {
                if (ReadFile(hCom, buf, sizeof(buf), &dwBytesRead, NULL)) {
                    //if (dwBytesRead == 1) break;
                    buf[dwBytesRead] = 0;
                    printf("req: %s\n", buf);
                    MaintReq(buf);
                }
            }
        }
        else {
            printf("Wait failed with error %d.\n", (int)GetLastError());
        }
    }

    CloseHandle(hCom);

    return 0;
}

int MaintReq(char *req) {
    int  len;
    DWORD BytesWritten;

    if (req[0] == CR) {
        WriteFile(hCom, &BELL, 1, &BytesWritten, NULL);
        return 0;
    }
    
    if (req[0] == 'N') {
        WriteFile(hCom, Serial, 6, &BytesWritten, NULL);
        return 0;
    }
    
    if (req[0] != 't') {
        WriteFile(hCom, &CR,    1, &BytesWritten, NULL);
        return 0;
    }
    
    WriteFile(hCom, &RSP, 1, &BytesWritten, NULL);
    WriteFile(hCom, &CR,  1, &BytesWritten, NULL);
    Sleep(50);

    req[3] = '3';

    if (req[6] == '4') Sleep(3500);

    if (req[6] == '2') {    // Query device - PIC18F25K80
        req[7]  = '6';
        req[8]  = '1';
        req[9]  = '8';
        req[10] = '0';
    }
    
    len = strlen(req);
    if (WriteFile(hCom, req, len, &BytesWritten, NULL))
    {
        printf("rsp: %s   len: %d   BytesWritten: %d\n", req, len, BytesWritten);
    }
    else
    {
       printf("WriteFile() failed with error %d.\n", (int)GetLastError());
    }
    
    return 0;
}
