/*
 *  H264 Encoder - Capture YUV, compress via VA-API and stream to RTP.
 *  Original code base was the vaapi h264encode application, with 
 *  significant additions to support capture, transform, compress
 *  and re-containering via libavformat.
 *
 *  Copyright (c) 2014-2017 Steven Toth <stoth@kernellabs.com>
 *  Copyright (c) 2014-2017 Zodiac Inflight Innovations
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

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
