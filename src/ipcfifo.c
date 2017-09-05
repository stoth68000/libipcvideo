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

#include <libipcvideo/ipcfifo.h>

void ipcfifo_init(struct ipcvideo_fifo_s *list)
{
	list->wptr = 0;
	list->rptr = 0;
	list->debug = 0;
	memset(list->item, 0, sizeof(list->item));
}

int ipcfifo_isempty(struct ipcvideo_fifo_s *list)
{
	int e = 0;

	if (list->rptr == list->wptr)
		e = 1;

	if (list->debug)
		printf("%s() = %d\n", __func__, e);

	return e;
}

void ipcfifo_push(struct ipcvideo_fifo_s *list, unsigned int value)
{
	list->item[ list->wptr++ ] = value;
	if (list->debug)
		printf("Pushing value %d\n", value);
	if (list->wptr == MAX_FIFO_ENTRIES)
		list->wptr = 0;

	/* If the fifo is now full, drop the last element */
	if (list->rptr == list->wptr) {
		if (list->debug)
			printf("Dropping item %d\n", list->item[ list->rptr ]);
		list->rptr++;
	} else
		list->count++;

	if (list->rptr == MAX_FIFO_ENTRIES)
		list->rptr = 0;

	if (list->debug)
		printf("rptr = %03d wptr = %03d\n", list->rptr, list->wptr);
}

/* return the top most item */
unsigned int ipcfifo_pop(struct ipcvideo_fifo_s *list)
{
	if (ipcfifo_isempty(list))
		return 0;

	unsigned int value = list->item[ list->rptr++ ];
	if (list->rptr == MAX_FIFO_ENTRIES)
		list->rptr = 0;

	list->count--;

	if (list->debug)
		printf("popping item %d\n", value);

	return value;
}

