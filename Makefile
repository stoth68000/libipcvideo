CC=gcc
AR=ar

CCFLAGS= -g -std=c99 -D_XOPEN_SOURCE
LIBS=-L. -lipcvideo -lpthread
INC=-I..
TARFILES=*.c *.h Makefile

all:	libipcvideo.a libipcvideo.so producer-demo consumer-demo fifo-demo metadata-demo

ipcfifo.o:	ipcfifo.c ipcfifo.h
	$(CC) $(CCFLAGS) $(INC) -c ipcfifo.c

ipcstatistics.o:	ipcstatistics.c ipcstatistics.h
	$(CC) $(CCFLAGS) $(INC) -c ipcstatistics.c

ipcvideo.o:	ipcvideo.c ipcvideo.h
	$(CC) $(CCFLAGS) $(INC) -c ipcvideo.c

ipcdisplay.o:	ipcdisplay.c ipcdisplay.h
	$(CC) $(CCFLAGS) $(INC) -c ipcdisplay.c

libipcvideo.a:	ipcvideo.o ipcfifo.o ipcstatistics.o ipcdisplay.o
	$(AR) -r libipcvideo.a ipcvideo.o ipcfifo.o ipcstatistics.o ipcdisplay.o

libipcvideo.so:	libipcvideo.a
	$(CC) -shared ipcvideo.o ipcfifo.o ipcstatistics.o ipcdisplay.o -o libipcvideo.so

producer-demo:	libipcvideo.a producer-demo.c
	$(CC) $(CCFLAGS) $(INC) $(@).c $(LIBS) -o $(@)

metadata-demo:	libipcvideo.a metadata-demo.c
	$(CC) $(CCFLAGS) $(INC) $(@).c $(LIBS) -o $(@)

consumer-demo:	producer-demo
	cp producer-demo consumer-demo

fifo-demo:	producer-demo
	cp producer-demo fifo-demo

clean:
	rm -f libipcvideo.a ipcvideo.o producer-demo consumer-demo fifo-demo ipcfifo.o ipcstatistics.o ipcdisplay.o metadata-demo
	rm -rf producer-demo.dSYM

install:
	mkdir -p /usr/include/libipcvideo
	cp -f ipcvideo.h /usr/include/libipcvideo
	cp -f ipcfifo.h /usr/include/libipcvideo
	cp -f ipcstatistics.h /usr/include/libipcvideo
	cp -f ipcdisplay.h /usr/include/libipcvideo
	cp -f libipcvideo.a /usr/lib
	cp -f libipcvideo.so /usr/lib

tarball: 
	cd .. && tar zcf libipcvideo-$(shell date +%Y%m%d-%H%M%S).tgz --exclude-vcs ./libipcvideo

