#ifndef SERIAL_H_INC
#define SERIAL_H_INC

extern int portOpen(char *, DWORD);
extern int portRead(char *, DWORD);
//extern int portRead(char *, DWORD, DWORD);
extern int portWrite(char *, DWORD);
extern int portClose(void);

extern int protInit(void);
extern int protOpen(char *);
extern int protClose(void);

#endif
