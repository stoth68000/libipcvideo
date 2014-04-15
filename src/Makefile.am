noinst_PROGRAMS = producer-demo metadata-demo fifo-demo consumer-demo
lib_LTLIBRARIES = libipcvideo.la

libipcvideo_includedir = $(includedir)/libipcvideo
libipcvideo_include_HEADERS = ipcvideo.h ipcfifo.h ipcstatistics.h ipcdisplay.h

libipcvideo_la_SOURCES = \
	ipcfifo.c \
	ipcstatistics.c \
	ipcvideo.c \
	ipcdisplay.c \
	$(include_HEADERS)

producer_demo_SOURCES = producer-demo.c
producer_demo_LDADD = libipcvideo.la @PTHREAD_LIBS@
fifo_demo_SOURCES = producer-demo.c
fifo_demo_LDADD = libipcvideo.la @PTHREAD_LIBS@
consumer_demo_SOURCES = producer-demo.c
consumer_demo_LDADD = libipcvideo.la @PTHREAD_LIBS@

metadata_demo_SOURCES = metadata-demo.c
metadata_demo_LDADD = libipcvideo.la @PTHREAD_LIBS@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libipcvideo.pc
