#ifndef IPCVIDEO_H
#define IPCVIDEO_H

/* Copyright 2014 Kernel Labs Inc. All Rights Reserved. */

/* Notes:
 * OSX, increase the maximum shared memory size to 256MB
 * sudo sysctl -w kern.sysv.shmmax=268435456
 * sudo sysctl -w kern.sysv.shmall=32768
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <libipcvideo/ipcfifo.h>
#include <libipcvideo/ipcstatistics.h>
#include <libipcvideo/ipcdisplay.h>

#define KLAPI_SUCCESS(nr) (nr >= 0)
#define KLAPI_FAILED(nr)  (nr <  0)
#define KLAPI_OK		 0
#define KLAPI_ERROR		-1
#define KLAPI_NOT_INITIALIZED	-2
#define KLAPI_BUSY		-3
#define KLAPI_INVALID_ARG	-4
#define KLAPI_NO_RESOURCE	-5
#define KLAPI_TIMEOUT		-6

#define IPCFOURCC_YUYV 0x59555956
#define IPCFOURCC_BGRX 0x42475258
#define IPCFOURCC_I420 0x49343230

struct ipcvideo_buffer_s;

/* Application specific context the library allocates */
struct ipcvideo_s {
	key_t         key;
	int           md_shmid;     /* shared memory segment id (metadata) */
	void          *md;          /* address of metadata struct */
	int           mdbuf_shmid;  /* shared memory segment id (bufhdrs and pixels) */
	unsigned char *mdbuf;       /* address of buffer headers and pixel data */
};

/* All buffers have the same video dimensions */
struct ipcvideo_dimensions_s {
	unsigned int width;
	unsigned int height;
	unsigned int depth;
	unsigned int stride;
	unsigned int fourcc;
} __attribute__ ((__packed__));

/* Create or destroy some basic application/library context */
int ipcvideo_context_create(struct ipcvideo_s **ctx);
int ipcvideo_context_destroy(struct ipcvideo_s *ctx);

/* Attach / detach to/from existing shared segments */
int ipcvideo_context_attach(struct ipcvideo_s *ctx, int id, const char *path, struct ipcvideo_dimensions_s *dimensions);
int ipcvideo_context_detach(struct ipcvideo_s *ctx);

/* Create the shared segments and any pixel buffers */
int ipcvideo_context_prepare(struct ipcvideo_s *ctx,
        int id, const char *path, struct ipcvideo_dimensions_s *dimensions, unsigned int buffers);

/* After the segments have been prepared, applications can query their dimensions */
int ipcvideo_context_get_dimensions(struct ipcvideo_s *ctx, struct ipcvideo_dimensions_s *d);

/* Buffer querying. Allow applications to get the pixel data buffer for filling / extraction. */
int ipcvideo_buffer_get_header(struct ipcvideo_s *ctx, unsigned int nr, struct ipcvideo_buffer_s **buf);
int ipcvideo_buffer_get_data(struct ipcvideo_s *ctx, struct ipcvideo_buffer_s *buf, unsigned char **data, unsigned int *length);

/* Buffer list management */
int ipcvideo_list_free_enqueue(struct ipcvideo_s *ctx, struct ipcvideo_buffer_s *buf);
int ipcvideo_list_busy_enqueue(struct ipcvideo_s *ctx, struct ipcvideo_buffer_s *buf);
int ipcvideo_list_free_dequeue(struct ipcvideo_s *ctx, struct ipcvideo_buffer_s **buf);
int ipcvideo_list_busy_dequeue(struct ipcvideo_s *ctx, struct ipcvideo_buffer_s **buf);
int ipcvideo_list_busy_isempty(struct ipcvideo_s *ctx, int *empty);
int ipcvideo_list_free_isempty(struct ipcvideo_s *ctx, int *empty);

/* Wait indefintely on a buffer being posted to the busy list */
int ipcvideo_list_busy_wait(struct ipcvideo_s *ctx);

/* Wait up to ms milliseconds for a buffer being posted to the busy list */
int ipcvideo_list_busy_timedwait(struct ipcvideo_s *ctx, unsigned int ms);

/* End -- Buffer list management */

/* Console dump / Debug */
int ipcvideo_dump_context(struct ipcvideo_s *ctx);
int ipcvideo_dump_metadata(struct ipcvideo_s *ctx);
int ipcvideo_dump_buffer(struct ipcvideo_s *ctx, struct ipcvideo_buffer_s *buf);
int ipcvideo_dump_buffers(struct ipcvideo_s *ctx);

/* Metadata related */

/* Library can update each frame with internal statistics, enable or disable this feature. */
int ipcvideo_metadata_set_osd(struct ipcvideo_s *ctx, int enable);

/* Reset the in/out frame counts */
int ipcvideo_statistics_reset(struct ipcvideo_s *ctx);

#endif // IPCVIDEO_H
