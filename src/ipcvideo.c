#include <libipcvideo/ipcvideo.h>

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/errno.h>

#define getMetadata(ctx) ((struct ipcvideo_md_s *)ctx->md)

#define MAX_IPC_BUFFERS 128

/* Buffer headers grouped together at the beginning of the
 * segment, followed by all the pixel data.
 * the offset can be used to determine the address of the first
 * pixel in the buffer.
 */
struct ipcvideo_buffer_s {
	unsigned int size;
	unsigned int nr;
	unsigned int offset;    /* md->buf + offset = first pixel */
	enum {
		BUF_UNDEFINED = 0,
		BUF_FREE,
		BUF_BUSY,
		BUF_LOST,
	} state;
	struct timespec tv_dequeue;
} __attribute__ ((__packed__));

struct ipcvideo_md_s {
	unsigned int    size;
	unsigned int    users;
	unsigned int    buffers;
	struct ipcvideo_dimensions_s dimensions;

	unsigned int    buflen; /* width * height * depth */

	/* Some none-address relative lists for managing buffer access between processes */
	pthread_mutex_t lockList;   /* Prevent multiple processes from touching the lists */
	pthread_mutexattr_t mutex_shared_attr;

	/* For each potential buffer (128 free / 128 busy). If valie == 0 then
	 * the buffer doesn't exist on this list. > 0 and it does.
	 * We're doing this as a static array which allows us to move the buffers between
	 * lists without using process specific addresses.
	 * We don't modify these lists without taking the lock mutex.
	 */
	struct ipcvideo_fifo_s freeList;
	struct ipcvideo_fifo_s busyList;

	/* Reader waiting... */
	pthread_mutex_t waiterLock;
	pthread_mutexattr_t waiterLockAttr;
	pthread_cond_t waiterBusyCond;
	pthread_condattr_t waiterBusyCondAttr;

	/* Statistics */
	struct ipcvideo_statistics_s stats;

#define MD_FLAGS_ENABLE_OSD 1
	unsigned int flags;

} __attribute__ ((__packed__));

static int isValidFourCC(unsigned int fourcc)
{
	switch(fourcc) {
	case IPCFOURCC_YUYV:
	case IPCFOURCC_BGRX:
	case IPCFOURCC_I420:
		return 1;
	default:
		return 0;
	}
}

#define TIMESPEC_AFTER(a, b) \
	(((a).tv_sec == (b).tv_sec) ?	\
	((a).tv_nsec > (b).tv_nsec) :	\
	((a).tv_sec > (b).tv_sec))

/* Generate the current time and add ms milliseconds to it */
static void timespec_add_ms(struct timespec *ts, int ms)
{
	unsigned long long us = ms * 1000000LL;

	/* Get the current time and add ms to is */
	struct timeval tv;
	gettimeofday(&tv, 0);

	/* Convert between structs */
	ts->tv_sec = tv.tv_sec;
	ts->tv_nsec = tv.tv_usec * 1000;

	/* Do the math and deal with the nsec wrap */
	ts->tv_sec += (us / 1000000000LL);
	ts->tv_nsec += (us % 1000000000LL);
	ts->tv_sec += (ts->tv_nsec / 1000000000LL);
	ts->tv_nsec %= 1000000000LL;
}

int ipcvideo_context_create(struct ipcvideo_s **ctx)
{
	struct ipcvideo_s *p = calloc(1, sizeof(struct ipcvideo_s));
	if (!p)
		return KLAPI_ERROR;

	*ctx = p;

	return KLAPI_OK;
}

int ipcvideo_context_destroy(struct ipcvideo_s *ctx)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	memset(ctx, 0, sizeof(*ctx));

	free(ctx);

	return KLAPI_OK;
}

/* Attach to an existing segment, assuming it exists */
int ipcvideo_context_attach(struct ipcvideo_s *ctx, int id, const char *path, struct ipcvideo_dimensions_s *dimensions)
{
	if ((!ctx) || (!path) || (id <= 0))
		return KLAPI_INVALID_ARG;

	key_t k = ftok(path, id);
	if (k == -1)
		return KLAPI_ERROR;

	/* Attach to segment if it already exists, don't attempt to create it. */
	int shmid = shmget(k, sizeof(struct ipcvideo_md_s), 0);
	if (shmid == -1)
		return KLAPI_NOT_INITIALIZED;

	ctx->key = k;
	ctx->md_shmid = shmid;

	/* Map the segment into out process space */
	ctx->md = shmat(shmid, 0, 0);
	if (!ctx->md)
		return KLAPI_NO_RESOURCE;

	/* Allocate the working buffers */
	struct ipcvideo_md_s *md = getMetadata(ctx);

	/* Allocate a large set of video buffers, in a single segment */
	key_t k2 = ftok(path, id + 1);
	if (k2 == -1)
		return KLAPI_ERROR;

	/* Create a new segment for all the pixel data and a buffer header for each */
	int shmid2 = shmget(k2, (md->buflen + sizeof(struct ipcvideo_buffer_s)) * md->buffers, 0);
	if (shmid2 == -1)
		return KLAPI_NO_RESOURCE;

	/* Map the segment into out process space */
	ctx->mdbuf = shmat(shmid2, 0, 0);
	if (!ctx->mdbuf)
		return KLAPI_ERROR;

	ctx->md_shmid = shmid2;
	md->users++;

	memcpy(dimensions, &md->dimensions, sizeof(md->dimensions));

	return KLAPI_OK;
}

/* Detach from an existing segment, assuming it exists, destroy everything if the segments
 * have no more users.
 */
int ipcvideo_context_detach(struct ipcvideo_s *ctx)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	printf("md->users = %d\n", md->users);
	md->users--;

	if (md->users == 0) {
		printf("detach and free\n");
		/* Wipe and detach the buffers */
		memset(ctx->mdbuf, 0, md->buffers * (md->buflen + sizeof(struct ipcvideo_buffer_s)));
		shmdt(ctx->mdbuf);
		shmctl(ctx->mdbuf_shmid, IPC_RMID, 0);
		ctx->mdbuf_shmid = 0;
		ctx->mdbuf = 0;
		md->buflen = 0;
		md->buffers = 0;
		md->buflen = 0;
		pthread_cond_destroy(&md->waiterBusyCond);
		memset(&md->dimensions, 0, sizeof(md->dimensions));

		/* Detach the metadata */
		shmdt(ctx->md);
		shmctl(ctx->md_shmid, IPC_RMID, 0);
	} else {
		/* Detach and don't destroy the buffers etc */
		printf("detach\n");
		shmdt(ctx->mdbuf);
		shmdt(ctx->md);

	}
	ctx->md_shmid = 0;
	ctx->md = 0;
	ctx->mdbuf_shmid = 0;
	ctx->mdbuf = 0;
	ctx->key = 0;

	return KLAPI_OK;
}

static void ipcvideo_context_force_removal(struct ipcvideo_s *ctx, int id, const char *path)
{
	/* Destroy the metadata, we cannot trust any of its content */
	key_t k = ftok(path, id);
	if (k != -1) {
		/* Get the existing segment */
		int shmid = shmget(k, 0, 0);
		if (shmid != -1) {
			/* Destroy it */
			shmctl(shmid, IPC_RMID, 0);
		}
	}

	/* Destroy the buffers */
	key_t k2 = ftok(path, id + 1);
	if (k2 != -1) {
		/* destroy a new segment for all the pixel data and a buffer header for each */
		int shmid = shmget(k2, 0, 0);
		if (shmid != -1) {
			/* Destroy it */
			shmctl(shmid, IPC_RMID, 0);
		}
	}
}

/* Allocate a new metadata segment, fill in th details, then allocate all the buffers.
 * in a seperate segment.
 */
int ipcvideo_context_prepare(struct ipcvideo_s *ctx, int id, const char *path, struct ipcvideo_dimensions_s *dimensions, unsigned int buffers)
{
	if ((!ctx) || (!path) || (id <= 0) || (!dimensions) || (!buffers) || (buffers < 4) || (buffers > MAX_IPC_BUFFERS))
		return KLAPI_INVALID_ARG;

	if ((!dimensions->width) || (!dimensions->height) || (!dimensions->depth) || (!dimensions->fourcc))
		return KLAPI_INVALID_ARG;

	if ((dimensions->width < 240) || (dimensions->width > 1920))
		return KLAPI_INVALID_ARG;

	if ((dimensions->height < 180) || (dimensions->height > 1080))
		return KLAPI_INVALID_ARG;

	if ((dimensions->depth == 0) || (dimensions->depth > 4))
		return KLAPI_INVALID_ARG;

	if ((dimensions->stride < dimensions->width) || (dimensions->stride > 8192 /* width * depth */))
		return KLAPI_INVALID_ARG;

	if (!isValidFourCC(dimensions->fourcc))
		return KLAPI_INVALID_ARG;

	if (ctx->md)
		return KLAPI_BUSY;

	/* Design rule:
	 * 1. Consumer will never be runing or be attached to the segments before the producer.
	 *    runit will take care of destroying the consumer if the producer terminates.
	 * 2. Destroy all segments prior to the producer creating them (in case of rare corruption).
	 */
	ipcvideo_context_force_removal(ctx, id, path);

	key_t k = ftok(path, id);
	if (k == -1)
		return KLAPI_ERROR;

	/* Create a new segment */
	int shmid = shmget(k, sizeof(struct ipcvideo_md_s), IPC_CREAT | 0660);
	if (shmid == -1)
		return KLAPI_NO_RESOURCE;

	/* Map the segment into out process space */
	ctx->md = shmat(shmid, 0, 0);
	if (!ctx->md)
		return KLAPI_NO_RESOURCE;

	ctx->key = k;
	ctx->md_shmid = shmid;

	/* Allocate the working buffers */
	struct ipcvideo_md_s *md = getMetadata(ctx);

	/* Prepare the metadata area */
	memset(md, 0, sizeof(struct ipcvideo_md_s));
	md->size = sizeof(struct ipcvideo_md_s);
	md->users = 1;
	memcpy(&md->dimensions, dimensions, sizeof(md->dimensions));
	md->buffers = buffers;
	if (dimensions->fourcc == IPCFOURCC_I420) {
		/* Y */
		md->buflen  = dimensions->width * dimensions->height;

		/* U + V */
		md->buflen += ((md->buflen / 4) * 2);
	} else
		md->buflen = dimensions->width * dimensions->height * dimensions->depth;

	ipcfifo_init(&md->freeList);
	ipcfifo_init(&md->busyList);
	ipcstatistics_reset(&md->stats);

	pthread_mutexattr_init(&md->mutex_shared_attr);
	pthread_mutexattr_setpshared(&md->mutex_shared_attr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&md->lockList, &md->mutex_shared_attr);

	/* Reader waiting mechanism */
	pthread_mutexattr_init(&md->waiterLockAttr);
	pthread_mutexattr_setpshared(&md->waiterLockAttr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&md->waiterLock, &md->waiterLockAttr);

	pthread_condattr_init(&md->waiterBusyCondAttr);
	pthread_condattr_setpshared(&md->waiterBusyCondAttr, PTHREAD_PROCESS_SHARED);
	pthread_cond_init(&md->waiterBusyCond, &md->waiterBusyCondAttr);

	/* Debugging features */
	md->flags = 0;

	/* Allocate a large set of video buffers, in a single segment */
	key_t k2 = ftok(path, id + 1);
	if (k2 == -1)
		return KLAPI_ERROR;

	/* Create a new segment for all the pixel data and a buffer header for each */
	int shmid2 = shmget(k2, (md->buflen + sizeof(struct ipcvideo_buffer_s)) * buffers, IPC_CREAT | 0660);
	if (shmid2 == -1)
		return KLAPI_NO_RESOURCE;

	/* Map the segment into out process space */
	ctx->mdbuf = shmat(shmid2, 0, 0);
	if (!ctx->mdbuf)
		return KLAPI_ERROR;

	ctx->mdbuf_shmid = shmid2;

	memset(ctx->mdbuf, 0, md->buflen * md->buffers);

	/* Prepare all the buffers headers */
	for (unsigned int i = 0; i < md->buffers; i++) {
		struct ipcvideo_buffer_s *b = 0;
		int ret = ipcvideo_buffer_get_header(ctx, i, &b);
		if (KLAPI_FAILED(ret)) {
			/* err */
			continue;
		}
		ipcvideo_dump_buffer(ctx, b);

		/* Prepare the buffer header */
		b->nr = i;
		b->size = sizeof(struct ipcvideo_buffer_s);
		b->state = BUF_FREE;

		/* offset to the first video pixel of this buffer, not it's address - which
		 * will differ between processes.
		 */
		b->offset = (md->buffers * sizeof(struct ipcvideo_buffer_s)) + (i * md->buflen);

		ipcvideo_list_free_enqueue(ctx, b);
	}

	return KLAPI_OK;
}

int ipcvideo_buffer_get_header(struct ipcvideo_s *ctx, unsigned int nr, struct ipcvideo_buffer_s **buf)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	if (nr >= md->buffers)
		return KLAPI_INVALID_ARG;

	*buf = (struct ipcvideo_buffer_s *)(ctx->mdbuf + (nr * sizeof(struct ipcvideo_buffer_s)));

	return KLAPI_OK;
}

int ipcvideo_list_busy_enqueue(struct ipcvideo_s *ctx, struct ipcvideo_buffer_s *buf)
{
	if ((!ctx) || (!buf))
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	do {
		/* Draw some statistics into the UI */
		if (!(md->flags & MD_FLAGS_ENABLE_OSD))
			break;

		struct ipcvideo_display_context dctx;
		int ret;
		ret = ipcvideo_display_init(&dctx, md->dimensions.fourcc);
		if (KLAPI_FAILED(ret))
			break;

		unsigned char *pixeldata;
		unsigned int length;
		ret = ipcvideo_buffer_get_data(ctx, buf, &pixeldata, &length);
		if (KLAPI_FAILED(ret))
			break;

		ret = ipcvideo_display_render_reset(&dctx, pixeldata, md->dimensions.stride);
		if (KLAPI_FAILED(ret))
			break;

		char str[256];
		sprintf(str, "in#: %3lld fps: %3d ms: %3d orun: %3lld",
			md->stats.in.totalFrames,
			md->stats.in.FPS,
			md->stats.in.latencyMs,
			md->stats.in.starvation);
		ret = ipcvideo_display_render_string(&dctx, (unsigned char*)str, strlen(str), 0, 10);
		if (KLAPI_FAILED(ret))
			break;

		sprintf(str, "out#: %3lld fps: %3d ms: %3d urun: %3lld lost: %lld",
			md->stats.out.totalFrames,
			md->stats.out.FPS,
			md->stats.out.latencyMs,
			md->stats.out.starvation,
			md->stats.lost_recycled);
		ret = ipcvideo_display_render_string(&dctx, (unsigned char*)str, strlen(str), 0, 11);
		if (KLAPI_FAILED(ret))
			break;

	} while (0);

	pthread_mutex_lock(&md->lockList);
	buf->state = BUF_BUSY;
	ipcfifo_push(&md->busyList, buf->nr);
	pthread_mutex_unlock(&md->lockList);

	ipcstatistics_frame_in(&md->stats);

	/* Notify any sleeping waiters a buffer needs processing */
	pthread_cond_broadcast(&md->waiterBusyCond);

	return KLAPI_OK;
}

int ipcvideo_list_busy_wait(struct ipcvideo_s *ctx)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	int empty;
	int ret = ipcvideo_list_busy_isempty(ctx, &empty);
	if (KLAPI_FAILED(ret))
		return ret;

	if (empty) {
		pthread_mutex_lock(&md->waiterLock);

		/* Wait will release the mutex but.... */
		pthread_cond_wait(&md->waiterBusyCond, &md->waiterLock);

		/* We return holding the mutex */
		pthread_mutex_unlock(&md->waiterLock);
	}

	return KLAPI_OK;
}

int ipcvideo_list_busy_timedwait(struct ipcvideo_s *ctx, unsigned int ms)
{
	if ((!ctx) || (ms > 5000))
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	int empty;
	int ret = ipcvideo_list_busy_isempty(ctx, &empty);
	if (KLAPI_FAILED(ret))
		return ret;

	ret = KLAPI_OK;
	if (empty) {
		pthread_mutex_lock(&md->waiterLock);

		struct timespec timeout;
		timespec_add_ms(&timeout, ms);

		/* Wait will release the mutex but.... */
		/* We return holding the mutex */
		ret = pthread_cond_timedwait(&md->waiterBusyCond, &md->waiterLock, &timeout);
		pthread_mutex_unlock(&md->waiterLock);

		if (ret == ETIMEDOUT) {
			/* No buffer within the time window */
			md->stats.out.starvation++;
			ret = KLAPI_TIMEOUT;
		} else
		if (ret == 0) {
			/* New busy buffer available */
			ret = KLAPI_OK;
		} else {
			/* Error */
			ret = KLAPI_ERROR;
		}

	}

	return ret;
}

int ipcvideo_list_free_enqueue(struct ipcvideo_s *ctx, struct ipcvideo_buffer_s *buf)
{
	if ((!ctx) || (!buf))
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	pthread_mutex_lock(&md->lockList);
	buf->state = BUF_FREE;
	ipcfifo_push(&md->freeList, buf->nr);
	pthread_mutex_unlock(&md->lockList);

	return KLAPI_OK;
}

/* This MUST be called with the mutex taken */
static void ipcvideo_list_coalesce(struct ipcvideo_s *ctx, struct ipcvideo_md_s *md)
{
	for (unsigned int i = 0; i < md->buffers; i++) {
		struct ipcvideo_buffer_s *buf = 0;
		int ret = ipcvideo_buffer_get_header(ctx, i, &buf);
		if (KLAPI_FAILED(ret)) {
			/* err */
			continue;
		}

		if (buf->state != BUF_LOST)
			continue;

		struct timespec now;
		timespec_add_ms(&now, 0);

		if (!TIMESPEC_AFTER(now, buf->tv_dequeue))
			continue;

		buf->state = BUF_FREE;
		ipcfifo_push(&md->freeList, buf->nr);

		md->stats.lost_recycled++;
	}
}

int ipcvideo_list_dequeue(struct ipcvideo_s *ctx, struct ipcvideo_fifo_s *list, struct ipcvideo_buffer_s **buf)
{
	struct ipcvideo_buffer_s *b = 0;

	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	pthread_mutex_lock(&md->lockList);

	/* If the lists have lost buffers, re-create the lists */
	ipcvideo_list_coalesce(ctx, md);

	if (!ipcfifo_isempty(list)) {
		unsigned int nr = ipcfifo_pop(list);
		ipcvideo_buffer_get_header(ctx, nr, &b);
		b->state = BUF_LOST;

		/* Put an expiration on this buffer, 3000ms.
		 * Its considered LOST by default and will
		 * be aggressively handled after timeout.
		 */
		timespec_add_ms(&b->tv_dequeue, 3000);
	}
	pthread_mutex_unlock(&md->lockList);

	*buf = b;
	return b ? KLAPI_OK : KLAPI_NO_RESOURCE;
}

int ipcvideo_list_busy_dequeue(struct ipcvideo_s *ctx, struct ipcvideo_buffer_s **buf)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	int ret = ipcvideo_list_dequeue(ctx, &md->busyList, buf);
	if (KLAPI_SUCCESS(ret))
		ipcstatistics_frame_out(&md->stats);
	else
		md->stats.out.starvation++;

	return ret;
}

int ipcvideo_list_free_dequeue(struct ipcvideo_s *ctx, struct ipcvideo_buffer_s **buf)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	int ret = ipcvideo_list_dequeue(ctx, &md->freeList, buf);
	if (KLAPI_FAILED(ret))
		md->stats.in.starvation++;

	return ret;
}

int ipcvideo_buffer_get_data(struct ipcvideo_s *ctx, struct ipcvideo_buffer_s *buf, unsigned char **data, unsigned int *length)
{
	if ((!ctx) || (!buf))
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	*data = ctx->mdbuf + buf->offset;
	*length = md->buflen;

	return KLAPI_OK;
}

int ipcvideo_dump_buffer(struct ipcvideo_s *ctx, struct ipcvideo_buffer_s *buf)
{
	if ((!ctx) || (!buf))
		return KLAPI_INVALID_ARG;

	unsigned char *addr = 0;
	unsigned int length;
	int ret = ipcvideo_buffer_get_data(ctx, buf, &addr, &length);
	if (KLAPI_FAILED(ret))
		return KLAPI_ERROR;

	printf("buffer[%p] - ", buf);
	printf("size 0x%x ", buf->size);
	printf("nr 0x%02x ", buf->nr);
	printf("offset 0x%08x ", buf->offset);
	printf("length 0x%08x ", length);
	printf("addr %p ", addr);
	printf("state %s\n",
		buf->state == BUF_UNDEFINED ? "UNDEFINED" :
		buf->state == BUF_FREE ? "FREE" :
		buf->state == BUF_LOST ? "LOST" :
		buf->state == BUF_BUSY ? "BUSY" : "?");

	return KLAPI_OK;
}

int ipcvideo_dump_metadata(struct ipcvideo_s *ctx)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	printf("metadata[%p]\n", md);
	printf(" .size    = 0x%x\n", md->size);
	printf(" .users   = %d\n", md->users);
	printf(" .buffers = 0x%x\n", md->buffers);
	printf(" .width   = %d\n", md->dimensions.width);
	printf(" .height  = %d\n", md->dimensions.height);
	printf(" .depth   = %d\n", md->dimensions.depth);
	printf(" .stride  = %d\n", md->dimensions.stride);
	printf(" .fourcc  = 0x%x\n", md->dimensions.fourcc);
	printf(" .buflen  = 0x%x\n", md->buflen);
	printf(" .freeCnt = %d\n", md->freeList.count);
	printf(" .busyCnt = %d\n", md->busyList.count);
	printf(" .lost    = %lld\n", md->stats.lost_recycled);
	printf(" .stats\n");
	printf("  .frames_in  = %lld\n", md->stats.in.totalFrames);
	printf("   .latency   = %d (ms)\n", md->stats.in.latencyMs);
	printf("   .fps       = %d\n", md->stats.in.FPS);
	printf("   .underrun  = %lld\n", md->stats.in.starvation);
	printf("  .frames_out = %lld\n", md->stats.out.totalFrames);
	printf("   .latency   = %d (ms)\n", md->stats.out.latencyMs);
	printf("   .fps       = %d\n", md->stats.out.FPS);
	printf("   .underrun  = %lld\n", md->stats.out.starvation);
	printf(" .flags   = %08x %s\n", md->flags,
		md->flags & MD_FLAGS_ENABLE_OSD ? "ENABLE_OSD" : "");

	return KLAPI_OK;
}

int ipcvideo_dump_context(struct ipcvideo_s *ctx)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	printf("context[%p]\n", ctx);
	printf(" .key         = 0x%x\n", ctx->key);
	printf(" .md_shmid    = %d\n", ctx->md_shmid);
	printf(" .md          = %p\n", ctx->md);
	printf(" .mdbuf_shmid = %d\n", ctx->mdbuf_shmid);
	printf(" .mdbuf       = %p\n", ctx->mdbuf);

	return KLAPI_OK;
}

int ipcvideo_dump_buffers(struct ipcvideo_s *ctx)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	for (unsigned int i = 0; i < md->buffers; i++) {
		struct ipcvideo_buffer_s *buf;
		int ret = ipcvideo_buffer_get_header(ctx, i, &buf);
		if (KLAPI_FAILED(ret))
			continue;
		
		ipcvideo_dump_buffer(ctx, buf);
	}
	return KLAPI_OK;
}

int ipcvideo_list_busy_isempty(struct ipcvideo_s *ctx, int *empty)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	*empty = ipcfifo_isempty(&md->busyList);

	return KLAPI_OK;
}

int ipcvideo_list_free_isempty(struct ipcvideo_s *ctx, int *empty)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	*empty = ipcfifo_isempty(&md->freeList);

	return KLAPI_OK;
}

int ipcvideo_context_get_dimensions(struct ipcvideo_s *ctx, struct ipcvideo_dimensions_s *d)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	memcpy(d, &md->dimensions, sizeof(md->dimensions));

	return KLAPI_OK;
}

int ipcvideo_metadata_set_osd(struct ipcvideo_s *ctx, int enable)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	if (!isValidFourCC(md->dimensions.fourcc))
		return KLAPI_ERROR;

	if (enable)
		md->flags |= MD_FLAGS_ENABLE_OSD;
	else
		md->flags &= ~MD_FLAGS_ENABLE_OSD;

	return KLAPI_OK;
}

int ipcvideo_statistics_reset(struct ipcvideo_s *ctx)
{
	if (!ctx)
		return KLAPI_INVALID_ARG;

	struct ipcvideo_md_s *md = getMetadata(ctx);
	if (!md)
		return KLAPI_NOT_INITIALIZED;

	ipcstatistics_reset(&md->stats);

	return KLAPI_OK;
}

const char *
ipcvideo_get_version(void)
{
    static const char *version_str = PACKAGE_VERSION;
    return version_str;
}
