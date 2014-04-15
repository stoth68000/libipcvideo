#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <libipcvideo/ipcvideo.h>

int g_running = 1;

extern int usleep (__useconds_t __useconds);

void signal_handler(int signum)
{
	printf("\n%s() Caught signal %d\n", __func__, signum);
	g_running = 0;
}

int fifo_main(int argc, char *argv[])
{
	struct ipcvideo_fifo_s fifo;

	ipcfifo_init(&fifo);

	for (unsigned int i = 0; i < 280; i++)
		ipcfifo_push(&fifo, i);

	while (!ipcfifo_isempty(&fifo)) {
		printf("%d\n", ipcfifo_pop(&fifo));
	}
}

int producer_main(int argc, char *argv[])
{
	struct ipcvideo_s *ctx = 0;

	printf("%s() begins\n", __func__);

	int ret = ipcvideo_context_create(&ctx);
	if (KLAPI_FAILED(ret)) {
		printf("Failed to create a context\n");
		return -1;
	}

	ipcvideo_dump_context(ctx);

	/* Attach to a segment, get its working dimensions */
	struct ipcvideo_dimensions_s d;
	ret = ipcvideo_context_attach(ctx, 1999, "/tmp", &d);
	if (KLAPI_FAILED(ret)) {
		printf("Unable to attach to segment, creating it.\n");

		/* Create the dimensions and segment */
		d.width = 1280;
		d.height = 768;
		d.depth = 4;
		d.stride = d.width;
		d.fourcc = IPCFOURCC_YUYV;
		ret = ipcvideo_context_prepare(ctx, 1999, "/tmp", &d, 4);
		if (KLAPI_FAILED(ret)) {
			printf("Unable to prepare segment err = %d, aborting.\n", ret);
			if (ret == KLAPI_NO_RESOURCE)
				printf("Increase shmax is larger enough to hold all your buffers\n");
			goto out;
		}
	}

	ipcvideo_dump_context(ctx);
	ipcvideo_dump_metadata(ctx);
	ipcvideo_dump_buffers(ctx);

	/* Do something with the buffers */
	signal(SIGINT, signal_handler);
	g_running = 1;
	int free;
	while (g_running) {
		usleep(100 * 1000);
		free = 1;

		struct ipcvideo_buffer_s *buf = 0;
		ret = ipcvideo_list_free_dequeue(ctx, &buf);
		if (KLAPI_FAILED(ret)) {

			/* Free list has stalled? Pop the busy list and use it */
			ret = ipcvideo_list_busy_dequeue(ctx, &buf);
			if (KLAPI_FAILED(ret))
				continue;

			free = 0;
		}

		unsigned char *pixels;
		unsigned int length;
		ret = ipcvideo_buffer_get_data(ctx, buf, &pixels, &length);
		if (KLAPI_FAILED(ret)) {
			ipcvideo_list_free_enqueue(ctx, buf);
			continue;
		}

		printf("Got %s buffer %p length %x [ %02x %02x %02x %02x ]\n",
			free ? "free" : "recycled busy",
			pixels, length,
			*(pixels + 0),
			*(pixels + 1),
			*(pixels + 2),
			*(pixels + 3));
		memset(pixels, 0xfe, length);
		ipcvideo_list_busy_enqueue(ctx, buf);
	}

	ret = ipcvideo_context_detach(ctx);
	if (KLAPI_FAILED(ret)) {
		printf("Unable to detach err = %d, aborting.\n", ret);
		goto out;
	}
	ipcvideo_dump_context(ctx);

out:
	ret = ipcvideo_context_destroy(ctx);
	if (KLAPI_FAILED(ret)) {
		printf("Failed to destroy a context\n");
		return -1;
	}

	return 0;
}

int consumer_main(int argc, char *argv[])
{
	struct ipcvideo_s *ctx = 0;

	printf("%s() begins\n", __func__);

	int ret = ipcvideo_context_create(&ctx);
	if (KLAPI_FAILED(ret)) {
		printf("Failed to create a context\n");
		return -1;
	}

	ipcvideo_dump_context(ctx);

	/* Attach to a segment, get its working dimensions */
	struct ipcvideo_dimensions_s d;
	ret = ipcvideo_context_attach(ctx, 1999, "/tmp", &d);
	if (KLAPI_FAILED(ret)) {
		printf("Unable to attach to segment, aborting.\n");
		goto out;
	}

	printf("Attached to ipcvideo segment, dimensions %dx%d fourcc: %08x\n",
		d.width, d.height, d.fourcc);

	ipcvideo_dump_context(ctx);
	ipcvideo_dump_metadata(ctx);
	ipcvideo_dump_buffers(ctx);

	/* Do something with the buffers */
	signal(SIGINT, signal_handler);

	struct ipcvideo_buffer_s *lastBuffer = 0;
	g_running = 1;
	while (g_running) {
		struct ipcvideo_buffer_s *buf = 0;

		ret = ipcvideo_list_busy_timedwait(ctx, 80);
		if (ret == KLAPI_TIMEOUT) {
			printf("cond failed\n");

			/* Use the previous buffer is we have one. */
			if (lastBuffer == 0)
				continue;

			buf = lastBuffer;
		} else {
			ret = ipcvideo_list_busy_dequeue(ctx, &buf);
			if (KLAPI_FAILED(ret)) {
				printf("1\n");
				continue;
			}
		}

		/* Requeue the previous buffer if we have a newer buffer */
		if (buf && lastBuffer && (lastBuffer != buf)) {
			ipcvideo_list_free_enqueue(ctx, lastBuffer);
			lastBuffer = 0;
		}

		unsigned char *pixels;
		unsigned int length;
		ret = ipcvideo_buffer_get_data(ctx, buf, &pixels, &length);
		if (KLAPI_FAILED(ret)) {
			ipcvideo_list_busy_enqueue(ctx, buf);
			continue;
		}

		printf("Got busy buffer %p length %x [ %02x %02x %02x %02x ]\n", pixels, length,
			*(pixels + 0),
			*(pixels + 1),
			*(pixels + 2),
			*(pixels + 3));

		/* Do something with it */

		lastBuffer = buf;
	}

	ret = ipcvideo_context_detach(ctx);
	if (KLAPI_FAILED(ret)) {
		printf("Unable to detach err = %d, aborting.\n", ret);
		goto out;
	}
	ipcvideo_dump_context(ctx);

out:
	ret = ipcvideo_context_destroy(ctx);
	if (KLAPI_FAILED(ret)) {
		printf("Failed to destroy a context\n");
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if ((strcmp(argv[0], "producer-demo") == 0) || (strcmp(argv[0], "./producer-demo") == 0))
		return producer_main(argc, argv);
	else
	if ((strcmp(argv[0], "consumer-demo") == 0) || (strcmp(argv[0], "./consumer-demo") == 0))
		return consumer_main(argc, argv);
	else
	if ((strcmp(argv[0], "fifo-demo") == 0) || (strcmp(argv[0], "./fifo-demo") == 0))
		return fifo_main(argc, argv);
	else
		printf("What?\n");

	return -1;
}
