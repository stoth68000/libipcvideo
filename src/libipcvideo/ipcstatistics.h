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

#ifndef IPCSTATISTICS_H
#define IPCSTATISTICS_H

#include <stdio.h>
#include <sys/time.h>

struct ipcvideo_statistics_s
{
	struct {
        	unsigned long long totalFrames;
		unsigned int latencyMs;
		unsigned int FPS;
        	unsigned long long starvation;
	} in, out;
	unsigned long long lost_recycled;

	struct timeval timeslots[16];
} __attribute__ ((__packed__));

void ipcstatistics_reset(struct ipcvideo_statistics_s *stats);
void ipcstatistics_frame_in(struct ipcvideo_statistics_s *stats);
void ipcstatistics_frame_out(struct ipcvideo_statistics_s *stats);

#endif
