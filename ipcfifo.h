
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
