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

#ifndef IPCDISPLAY_H
#define IPCDISPLAY_H

/* Copyright 2014 Kernel Labs Inc. All Rights Reserved. */

#include "ipcvideo.h"

struct ipcvideo_display_context
{
	int plotwidth;
	int plotheight;
	int currx;
	int curry;
	int stride;

	unsigned char *ptr;
	unsigned char *frame;

	unsigned char bg[2], fg[2];

	unsigned int mode;
};

int ipcvideo_display_init(struct ipcvideo_display_context *ctx, unsigned int mode);
int ipcvideo_display_render_string(struct ipcvideo_display_context *ctx, unsigned char *s, unsigned int len, unsigned int x, unsigned int y);
int ipcvideo_display_render_reset(struct ipcvideo_display_context *ctx, unsigned char *ptr, unsigned int stride);

#endif
