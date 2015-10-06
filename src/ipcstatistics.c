#include <stdio.h>
#include <string.h>

#include <libipcvideo/ipcstatistics.h>

void ipcstatistics_reset(struct ipcvideo_statistics_s *stats)
{
	if (!stats)
		return;

	stats->in.totalFrames = 0;
	stats->in.starvation = 0;

	stats->out.totalFrames = 0;
	stats->out.starvation = 0;

	stats->lost_recycled = 0;
}

static void measureBegin(struct ipcvideo_statistics_s *stats, int slotnr)
{
	if (slotnr >= 16)
		return;

	gettimeofday(&stats->timeslots[slotnr], 0);
}

static unsigned int measureNow(struct ipcvideo_statistics_s *stats, int slotnr)
{
	if (slotnr >= 16)
		return 0;

	struct timeval now;
	gettimeofday(&now, 0);

	unsigned int elapsedTime = (now.tv_sec - stats->timeslots[slotnr].tv_sec) * 1000.0; /* sec to ms */
        elapsedTime += (now.tv_usec - stats->timeslots[slotnr].tv_usec) / 1000.0;  /* us to ms */

	return elapsedTime;
}

void ipcstatistics_frame_in(struct ipcvideo_statistics_s *stats)
{
	if (!stats)
		return;

	stats->in.totalFrames++;

	/* Measure interval between inframe calls */
	stats->in.latencyMs = measureNow(stats, 0);

	/* Avoid div by zero */
	if (stats->in.latencyMs)
		stats->in.FPS = (1000 / stats->in.latencyMs);
	else
		stats->in.FPS = 0;
	measureBegin(stats, 0); /* Slot 0, measure in frame latency */
}

void ipcstatistics_frame_out(struct ipcvideo_statistics_s *stats)
{
	if (!stats)
		return;

	stats->out.totalFrames++;

	/* Measure interval between outframe calls */
	stats->out.latencyMs = measureNow(stats, 1);

	if (stats->out.latencyMs)
		stats->out.FPS = (1000 / stats->out.latencyMs);
	else
		stats->out.FPS = 0;
	measureBegin(stats, 1); /* Slot 1, measure out frame latency */
}

