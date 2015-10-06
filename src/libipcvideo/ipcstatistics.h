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
