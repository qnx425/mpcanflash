VERSION_MAIN = 1
VERSION_SUB  = 8

CC    = gcc
EXECS = mpcanflash.exe
OBJS  = main.o hex.o usb-windows.o DeviceData.o serial.o
CFLAGS = -Wall -DWIN -DVERSION_MAIN=$(VERSION_MAIN) -DVERSION_SUB=$(VERSION_SUB)
# -DDEBUG

all: $(EXECS)

*.o: mpcanflash.h

.c.o:
	$(CC) $(CFLAGS) -c $*.c

# todo: add 64 bit target (low priority as 32 bit works on all platforms)

mpcanflash32: CFLAGS += -m32 
mpcanflash32: LDFLAGS += -m32
mpcanflash32: mpcanflash.exe

$(EXECS): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o bin/mpcanflash.exe

clean:
	del *.o


