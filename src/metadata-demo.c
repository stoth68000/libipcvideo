#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <libipcvideo/ipcvideo.h>

static void metadata_usage()
{
	printf("-o <0|1>   (def:0 Disable(0) or Enable(1) the OSD)\n");
	printf("-r         (      Reset frame in/out statistics)\n");
}

int main(int argc, char *argv[])
{
	struct ipcvideo_s *ctx = 0;
	int opt;

	int osd_update = 0, osd_val = 0;
	int stats_reset = 0;

	while ((opt = getopt(argc, argv, "o:r"))!= -1) {
		switch(opt) {
		case 'o':
			osd_update++;
			osd_val = atoi(optarg);
			printf("updating osd display = %s\n", osd_val ? "enabled" : "disabled");
			break;
		case 'r':
			stats_reset++;
			break;
		default:
			metadata_usage();
			exit(1);
		case -1:
			break;
                }
        }

	int ret = ipcvideo_context_create(&ctx);
	if (KLAPI_FAILED(ret)) {
		printf("Failed to create a context\n");
		return -1;
	}

	/* Attach to a segment, get its working dimensions */
	struct ipcvideo_dimensions_s d;
	ret = ipcvideo_context_attach(ctx, 1999, "/tmp", &d);
	if (KLAPI_FAILED(ret)) {
		printf("Unable to attach to segment, aborting.\n");
		goto out;
	}

	printf("Attached to ipcvideo segment, dimensions %dx%d fourcc: %08x\n",
		d.width, d.height, d.fourcc);

	if (osd_update && osd_val)
		ipcvideo_metadata_set_osd(ctx, 1);
	if (osd_update && !osd_val)
		ipcvideo_metadata_set_osd(ctx, 0);
	if (stats_reset)
		ipcvideo_statistics_reset(ctx);

	ipcvideo_dump_metadata(ctx);

	ret = ipcvideo_context_detach(ctx);
	if (KLAPI_FAILED(ret)) {
		printf("Unable to detach err = %d, aborting.\n", ret);
	}

out:
	ret = ipcvideo_context_destroy(ctx);
	if (KLAPI_FAILED(ret)) {
		printf("Failed to destroy a context\n");
		return -1;
	}

	return 0;
}
