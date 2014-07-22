
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

	enum {
		MODE_YUY2,
		MODE_BGRX,
	} mode;
};

int ipcvideo_display_init(struct ipcvideo_display_context *ctx);
int ipcvideo_display_render_string(struct ipcvideo_display_context *ctx, unsigned char *s, unsigned int len, unsigned int x, unsigned int y);
int ipcvideo_display_render_reset(struct ipcvideo_display_context *ctx, unsigned char *ptr, unsigned int stride);

#endif
