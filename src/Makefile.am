bin_PROGRAMS = producer-demo metadata-demo fifo-demo consumer-demo
lib_LTLIBRARIES = libipcvideo.la

libipcvideo_includedir = $(includedir)/libipcvideo
libipcvideo_include_HEADERS = \
	libipcvideo/ipcvideo.h \
	libipcvideo/ipcfifo.h \
	libipcvideo/ipcstatistics.h \
	libipcvideo/ipcdisplay.h

libipcvideo_la_SOURCES = \
	ipcfifo.c \
	ipcstatistics.c \
	ipcvideo.c \
	ipcdisplay.c \
	$(include_HEADERS)
libipcvideo_la_CFLAGS = @PTHREAD_CFLAGS@
libipcvideo_la_LIBADD = @PTHREAD_LIBS@

producer_demo_SOURCES = producer-demo.c
producer_demo_LDADD = libipcvideo.la
fifo_demo_SOURCES = producer-demo.c
fifo_demo_LDADD = libipcvideo.la
consumer_demo_SOURCES = producer-demo.c
consumer_demo_LDADD = libipcvideo.la

metadata_demo_SOURCES = metadata-demo.c
metadata_demo_LDADD = libipcvideo.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libipcvideo.pc
