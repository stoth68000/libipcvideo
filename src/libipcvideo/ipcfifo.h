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

#ifndef IPCFIFO_H
#define IPCFIFO_H

#include <stdio.h>

#define MAX_FIFO_ENTRIES 132

/* A non-memory address based list mechanism that can be used
 * between processes in shared memory segments.
 * The fifo contains buffer index numbers.
 */
struct ipcvideo_fifo_s {
        unsigned int item[MAX_FIFO_ENTRIES];
        int rptr, wptr;
	unsigned int count;
	int debug;
} __attribute__ ((__packed__));

void ipcfifo_init(struct ipcvideo_fifo_s *list);
int  ipcfifo_isempty(struct ipcvideo_fifo_s *list);
void ipcfifo_push(struct ipcvideo_fifo_s *list, unsigned int value);
unsigned int ipcfifo_pop(struct ipcvideo_fifo_s *list);

#endif
