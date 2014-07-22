/* Copyright 2014 Kernel Labs Inc. All Rights Reserved. */

#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include <libipcvideo/ipcdisplay.h>

#define ALPHA_BACKGROUND 0

static struct letter_t {
	unsigned char *ptr;
	unsigned char data[8];
} charset[] = 
{
 /* ' ' */ [0x20] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '!' */ [0x21] = { 0, { 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x04, 0x00 }, },
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000000 */
 /* 00000000 */
 /* 00000100 */
 /* 00000000 */
 /* '"' */ [0x22] = { 0, { 0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00001010 */
 /* 00001010 */
 /* 00001010 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '#' */ [0x23] = { 0, { 0x0a, 0x0a, 0x1f, 0x0a, 0x1f, 0x0a, 0x0a, 0x00 }, },
 /* 00001010 */
 /* 00001010 */
 /* 00011111 */
 /* 00001010 */
 /* 00011111 */
 /* 00001010 */
 /* 00001010 */
 /* 00000000 */
 /* '$' */ [0x24] = { 0, { 0x04, 0x0f, 0x14, 0x0e, 0x05, 0x1e, 0x04, 0x00 }, },
 /* 00000100 */
 /* 00001111 */
 /* 00010100 */
 /* 00001110 */
 /* 00000101 */
 /* 00011110 */
 /* 00000100 */
 /* 00000000 */
 /* '%' */ [0x25] = { 0, { 0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03, 0x00 }, },
 /* 00011000 */
 /* 00011001 */
 /* 00000010 */
 /* 00000100 */
 /* 00001000 */
 /* 00010011 */
 /* 00000011 */
 /* 00000000 */
 /* '&' */ [0x26] = { 0, { 0x0c, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0d, 0x00 }, },
 /* 00001100 */
 /* 00010010 */
 /* 00010100 */
 /* 00001000 */
 /* 00010101 */
 /* 00010010 */
 /* 00001101 */
 /* 00000000 */
 /* ''' */ [0x27] = { 0, { 0x0c, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00001100 */
 /* 00000100 */
 /* 00001000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '(' */ [0x28] = { 0, { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02, 0x00 }, },
 /* 00000010 */
 /* 00000100 */
 /* 00001000 */
 /* 00001000 */
 /* 00001000 */
 /* 00000100 */
 /* 00000010 */
 /* 00000000 */
 /* ')' */ [0x29] = { 0, { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08, 0x00 }, },
 /* 00001000 */
 /* 00000100 */
 /* 00000010 */
 /* 00000010 */
 /* 00000010 */
 /* 00000100 */
 /* 00001000 */
 /* 00000000 */
 /* '*' */ [0x2a] = { 0, { 0x00, 0x04, 0x15, 0x0e, 0x15, 0x04, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000100 */
 /* 00010101 */
 /* 00001110 */
 /* 00010101 */
 /* 00000100 */
 /* 00000000 */
 /* 00000000 */
 /* '+' */ [0x2b] = { 0, { 0x00, 0x04, 0x04, 0x1f, 0x04, 0x04, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000100 */
 /* 00000100 */
 /* 00011111 */
 /* 00000100 */
 /* 00000100 */
 /* 00000000 */
 /* 00000000 */
 /* ',' */ [0x2c] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x0c, 0x04, 0x08, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00001100 */
 /* 00000100 */
 /* 00001000 */
 /* 00000000 */
 /* '-' */ [0x2d] = { 0, { 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00011111 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '.' */ [0x2e] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00001100 */
 /* 00001100 */
 /* 00000000 */
 /* '/' */ [0x2f] = { 0, { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000001 */
 /* 00000010 */
 /* 00000100 */
 /* 00001000 */
 /* 00010000 */
 /* 00000000 */
 /* 00000000 */
 /* '0' */ [0x30] = { 0, { 0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e, 0x00 }, },
 /* 00001110 */
 /* 00010001 */
 /* 00010011 */
 /* 00010101 */
 /* 00011001 */
 /* 00010001 */
 /* 00001110 */
 /* 00000000 */
 /* '1' */ [0x31] = { 0, { 0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e, 0x00 }, },
 /* 00000100 */
 /* 00001100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00001110 */
 /* 00000000 */
 /* '2' */ [0x32] = { 0, { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f, 0x00 }, },
 /* 00001110 */
 /* 00010001 */
 /* 00000001 */
 /* 00000010 */
 /* 00000100 */
 /* 00001000 */
 /* 00011111 */
 /* 00000000 */
 /* '3' */ [0x33] = { 0, { 0x1f, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0e, 0x00 }, },
 /* 00011111 */
 /* 00000010 */
 /* 00000100 */
 /* 00000010 */
 /* 00000001 */
 /* 00010001 */
 /* 00001110 */
 /* 00000000 */
 /* '4' */ [0x34] = { 0, { 0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02, 0x00 }, },
 /* 00000010 */
 /* 00000110 */
 /* 00001010 */
 /* 00010010 */
 /* 00011111 */
 /* 00000010 */
 /* 00000010 */
 /* 00000000 */
 /* '5' */ [0x35] = { 0, { 0x1f, 0x10, 0x1e, 0x01, 0x01, 0x11, 0x0e, 0x00 }, },
 /* 00011111 */
 /* 00010000 */
 /* 00011110 */
 /* 00000001 */
 /* 00000001 */
 /* 00010001 */
 /* 00001110 */
 /* 00000000 */
 /* '6' */ [0x36] = { 0, { 0x06, 0x08, 0x10, 0x1e, 0x11, 0x11, 0x0e, 0x00 }, },
 /* 00000110 */
 /* 00001000 */
 /* 00010000 */
 /* 00011110 */
 /* 00010001 */
 /* 00010001 */
 /* 00001110 */
 /* 00000000 */
 /* '7' */ [0x37] = { 0, { 0x1f, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04, 0x00 }, },
 /* 00011111 */
 /* 00000001 */
 /* 00000010 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000000 */
 /* '8' */ [0x38] = { 0, { 0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e, 0x00 }, },
 /* 00001110 */
 /* 00010001 */
 /* 00010001 */
 /* 00001110 */
 /* 00010001 */
 /* 00010001 */
 /* 00001110 */
 /* 00000000 */
 /* '9' */ [0x39] = { 0, { 0x0e, 0x11, 0x11, 0x0f, 0x01, 0x02, 0x0c, 0x00 }, },
 /* 00001110 */
 /* 00010001 */
 /* 00010001 */
 /* 00001111 */
 /* 00000001 */
 /* 00000010 */
 /* 00001100 */
 /* 00000000 */
 /* ':' */ [0x3a] = { 0, { 0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00001100 */
 /* 00001100 */
 /* 00000000 */
 /* 00001100 */
 /* 00001100 */
 /* 00000000 */
 /* 00000000 */
 /* ';' */ [0x3b] = { 0, { 0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x04, 0x08, 0x00 }, },
 /* 00000000 */
 /* 00001100 */
 /* 00001100 */
 /* 00000000 */
 /* 00001100 */
 /* 00000100 */
 /* 00001000 */
 /* 00000000 */
 /* '<' */ [0x3c] = { 0, { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02, 0x00 }, },
 /* 00000010 */
 /* 00000100 */
 /* 00001000 */
 /* 00010000 */
 /* 00001000 */
 /* 00000100 */
 /* 00000010 */
 /* 00000000 */
 /* '=' */ [0x3d] = { 0, { 0x00, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00011111 */
 /* 00000000 */
 /* 00011111 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '>' */ [0x3e] = { 0, { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x00 }, },
 /* 00001000 */
 /* 00000100 */
 /* 00000010 */
 /* 00000001 */
 /* 00000010 */
 /* 00000100 */
 /* 00001000 */
 /* 00000000 */
 /* '?' */ [0x3f] = { 0, { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04, 0x00 }, },
 /* 00001110 */
 /* 00010001 */
 /* 00000001 */
 /* 00000010 */
 /* 00000100 */
 /* 00000000 */
 /* 00000100 */
 /* 00000000 */
 /* '@' */ [0x40] = { 0, { 0x0e, 0x11, 0x01, 0x0d, 0x15, 0x15, 0x0e, 0x00 }, },
 /* 00001110 */
 /* 00010001 */
 /* 00000001 */
 /* 00001101 */
 /* 00010101 */
 /* 00010101 */
 /* 00001110 */
 /* 00000000 */
 /* 'A' */ [0x41] = { 0, { 0x0e, 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x00 }, },
 /* 00001110 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00011111 */
 /* 00010001 */
 /* 00010001 */
 /* 00000000 */
 /* 'B' */ [0x42] = { 0, { 0x1e, 0x09, 0x09, 0x0e, 0x09, 0x09, 0x1e, 0x00 }, },
 /* 00011110 */
 /* 00001001 */
 /* 00001001 */
 /* 00001110 */
 /* 00001001 */
 /* 00001001 */
 /* 00011110 */
 /* 00000000 */
 /* 'C' */ [0x43] = { 0, { 0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e, 0x00 }, },
 /* 00001110 */
 /* 00010001 */
 /* 00010000 */
 /* 00010000 */
 /* 00010000 */
 /* 00010001 */
 /* 00001110 */
 /* 00000000 */
 /* 'D' */ [0x44] = { 0, { 0x1e, 0x09, 0x09, 0x09, 0x09, 0x09, 0x1e, 0x00 }, },
 /* 00011110 */
 /* 00001001 */
 /* 00001001 */
 /* 00001001 */
 /* 00001001 */
 /* 00001001 */
 /* 00011110 */
 /* 00000000 */
 /* 'E' */ [0x45] = { 0, { 0x1f, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x1f, 0x00 }, },
 /* 00011111 */
 /* 00010000 */
 /* 00010000 */
 /* 00011111 */
 /* 00010000 */
 /* 00010000 */
 /* 00011111 */
 /* 00000000 */
 /* 'F' */ [0x46] = { 0, { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10, 0x00 }, },
 /* 00011111 */
 /* 00010000 */
 /* 00010000 */
 /* 00011110 */
 /* 00010000 */
 /* 00010000 */
 /* 00010000 */
 /* 00000000 */
 /* 'G' */ [0x47] = { 0, { 0x0e, 0x11, 0x10, 0x13, 0x11, 0x11, 0x0f, 0x00 }, },
 /* 00001110 */
 /* 00010001 */
 /* 00010000 */
 /* 00010011 */
 /* 00010001 */
 /* 00010001 */
 /* 00001111 */
 /* 00000000 */
 /* 'H' */ [0x48] = { 0, { 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11, 0x00 }, },
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00011111 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00000000 */
 /* 'I' */ [0x49] = { 0, { 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e, 0x00 }, },
 /* 00001110 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00001110 */
 /* 00000000 */
 /* 'J' */ [0x4a] = { 0, { 0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0c, 0x00 }, },
 /* 00000111 */
 /* 00000010 */
 /* 00000010 */
 /* 00000010 */
 /* 00000010 */
 /* 00010010 */
 /* 00001100 */
 /* 00000000 */
 /* 'K' */ [0x4b] = { 0, { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11, 0x00 }, },
 /* 00010001 */
 /* 00010010 */
 /* 00010100 */
 /* 00011000 */
 /* 00010100 */
 /* 00010010 */
 /* 00010001 */
 /* 00000000 */
 /* 'L' */ [0x4c] = { 0, { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, 0x00 }, },
 /* 00010000 */
 /* 00010000 */
 /* 00010000 */
 /* 00010000 */
 /* 00010000 */
 /* 00010000 */
 /* 00011111 */
 /* 00000000 */
 /* 'M' */ [0x4d] = { 0, { 0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11, 0x00 }, },
 /* 00010001 */
 /* 00011011 */
 /* 00010101 */
 /* 00010101 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00000000 */
 /* 'N' */ [0x4e] = { 0, { 0x11, 0x19, 0x19, 0x15, 0x13, 0x13, 0x11, 0x00 }, },
 /* 00010001 */
 /* 00011001 */
 /* 00011001 */
 /* 00010101 */
 /* 00010011 */
 /* 00010011 */
 /* 00010001 */
 /* 00000000 */
 /* 'O' */ [0x4f] = { 0, { 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e, 0x00 }, },
 /* 00001110 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00001110 */
 /* 00000000 */
 /* 'P' */ [0x50] = { 0, { 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10, 0x00 }, },
 /* 00011110 */
 /* 00010001 */
 /* 00010001 */
 /* 00011110 */
 /* 00010000 */
 /* 00010000 */
 /* 00010000 */
 /* 00000000 */
 /* 'Q' */ [0x51] = { 0, { 0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x1d, 0x00 }, },
 /* 00001110 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00010101 */
 /* 00010010 */
 /* 00011101 */
 /* 00000000 */
 /* 'R' */ [0x52] = { 0, { 0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11, 0x00 }, },
 /* 00011110 */
 /* 00010001 */
 /* 00010001 */
 /* 00011110 */
 /* 00010100 */
 /* 00010010 */
 /* 00010001 */
 /* 00000000 */
 /* 'S' */ [0x53] = { 0, { 0x0e, 0x11, 0x10, 0x0e, 0x01, 0x11, 0x0e, 0x00 }, },
 /* 00001110 */
 /* 00010001 */
 /* 00010000 */
 /* 00001110 */
 /* 00000001 */
 /* 00010001 */
 /* 00001110 */
 /* 00000000 */
 /* 'T' */ [0x54] = { 0, { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00 }, },
 /* 00011111 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000000 */
 /* 'U' */ [0x55] = { 0, { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e, 0x00 }, },
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00001110 */
 /* 00000000 */
 /* 'V' */ [0x56] = { 0, { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04, 0x00 }, },
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00001010 */
 /* 00000100 */
 /* 00000000 */
 /* 'W' */ [0x57] = { 0, { 0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11, 0x00 }, },
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00010101 */
 /* 00010101 */
 /* 00011011 */
 /* 00010001 */
 /* 00000000 */
 /* 'X' */ [0x58] = { 0, { 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11, 0x00 }, },
 /* 00010001 */
 /* 00010001 */
 /* 00001010 */
 /* 00000100 */
 /* 00001010 */
 /* 00010001 */
 /* 00010001 */
 /* 00000000 */
 /* 'Y' */ [0x59] = { 0, { 0x11, 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x00 }, },
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00001010 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000000 */
 /* 'Z' */ [0x5a] = { 0, { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f, 0x00 }, },
 /* 00011111 */
 /* 00000001 */
 /* 00000010 */
 /* 00000100 */
 /* 00001000 */
 /* 00010000 */
 /* 00011111 */
 /* 00000000 */
 /* '[' */ [0x5b] = { 0, { 0x0e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0e, 0x00 }, },
 /* 00001110 */
 /* 00001000 */
 /* 00001000 */
 /* 00001000 */
 /* 00001000 */
 /* 00001000 */
 /* 00001110 */
 /* 00000000 */
 /* '\' */ [0x5c] = { 0, { 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00 }, },
 /* 00010000 */
 /* 00001000 */
 /* 00000100 */
 /* 00000010 */
 /* 00000001 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* ']' */ [0x5d] = { 0, { 0x0e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0e, 0x00 }, },
 /* 00001110 */
 /* 00000010 */
 /* 00000010 */
 /* 00000010 */
 /* 00000010 */
 /* 00000010 */
 /* 00001110 */
 /* 00000000 */
 /* '^' */ [0x5e] = { 0, { 0x04, 0x0a, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000100 */
 /* 00001010 */
 /* 00010001 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '_' */ [0x5f] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00011111 */
 /* 00000000 */
 /* '`' */ [0x60] = { 0, { 0x10, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00010000 */
 /* 00001000 */
 /* 00000100 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 'a' */ [0x61] = { 0, { 0x00, 0x00, 0x0e, 0x01, 0x0f, 0x11, 0x0f, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00001110 */
 /* 00000001 */
 /* 00001111 */
 /* 00010001 */
 /* 00001111 */
 /* 00000000 */
 /* 'b' */ [0x62] = { 0, { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x1e, 0x00 }, },
 /* 00010000 */
 /* 00010000 */
 /* 00010110 */
 /* 00011001 */
 /* 00010001 */
 /* 00010001 */
 /* 00011110 */
 /* 00000000 */
 /* 'c' */ [0x63] = { 0, { 0x00, 0x00, 0x0e, 0x11, 0x10, 0x11, 0x0e, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00001110 */
 /* 00010001 */
 /* 00010000 */
 /* 00010001 */
 /* 00001110 */
 /* 00000000 */
 /* 'd' */ [0x64] = { 0, { 0x01, 0x01, 0x0d, 0x13, 0x11, 0x11, 0x0f, 0x00 }, },
 /* 00000001 */
 /* 00000001 */
 /* 00001101 */
 /* 00010011 */
 /* 00010001 */
 /* 00010001 */
 /* 00001111 */
 /* 00000000 */
 /* 'e' */ [0x65] = { 0, { 0x00, 0x00, 0x0e, 0x11, 0x1f, 0x10, 0x0e, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00001110 */
 /* 00010001 */
 /* 00011111 */
 /* 00010000 */
 /* 00001110 */
 /* 00000000 */
 /* 'f' */ [0x66] = { 0, { 0x02, 0x05, 0x04, 0x0e, 0x04, 0x04, 0x04, 0x00 }, },
 /* 00000010 */
 /* 00000101 */
 /* 00000100 */
 /* 00001110 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000000 */
 /* 'g' */ [0x67] = { 0, { 0x00, 0x0d, 0x13, 0x13, 0x0d, 0x01, 0x0e, 0x00 }, },
 /* 00000000 */
 /* 00001101 */
 /* 00010011 */
 /* 00010011 */
 /* 00001101 */
 /* 00000001 */
 /* 00001110 */
 /* 00000000 */
 /* 'h' */ [0x68] = { 0, { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11, 0x00 }, },
 /* 00010000 */
 /* 00010000 */
 /* 00010110 */
 /* 00011001 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00000000 */
 /* 'i' */ [0x69] = { 0, { 0x04, 0x00, 0x0c, 0x04, 0x04, 0x04, 0x0e, 0x00 }, },
 /* 00000100 */
 /* 00000000 */
 /* 00001100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00001110 */
 /* 00000000 */
 /* 'j' */ [0x6a] = { 0, { 0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0c, 0x00 }, },
 /* 00000010 */
 /* 00000000 */
 /* 00000110 */
 /* 00000010 */
 /* 00000010 */
 /* 00010010 */
 /* 00001100 */
 /* 00000000 */
 /* 'k' */ [0x6b] = { 0, { 0x08, 0x08, 0x09, 0x0a, 0x0c, 0x0a, 0x09, 0x00 }, },
 /* 00001000 */
 /* 00001000 */
 /* 00001001 */
 /* 00001010 */
 /* 00001100 */
 /* 00001010 */
 /* 00001001 */
 /* 00000000 */
 /* 'l' */ [0x6c] = { 0, { 0x0c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e, 0x00 }, },
 /* 00001100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00001110 */
 /* 00000000 */
 /* 'm' */ [0x6d] = { 0, { 0x00, 0x00, 0x1a, 0x15, 0x15, 0x15, 0x15, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00011010 */
 /* 00010101 */
 /* 00010101 */
 /* 00010101 */
 /* 00010101 */
 /* 00000000 */
 /* 'n' */ [0x6e] = { 0, { 0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00010110 */
 /* 00011001 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00000000 */
 /* 'o' */ [0x6f] = { 0, { 0x00, 0x00, 0x0e, 0x11, 0x11, 0x11, 0x0e, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00001110 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00001110 */
 /* 00000000 */
 /* 'p' */ [0x70] = { 0, { 0x00, 0x16, 0x19, 0x19, 0x16, 0x10, 0x10, 0x00 }, },
 /* 00000000 */
 /* 00010110 */
 /* 00011001 */
 /* 00011001 */
 /* 00010110 */
 /* 00010000 */
 /* 00010000 */
 /* 00000000 */
 /* 'q' */ [0x71] = { 0, { 0x00, 0x0d, 0x13, 0x13, 0x0d, 0x01, 0x01, 0x00 }, },
 /* 00000000 */
 /* 00001101 */
 /* 00010011 */
 /* 00010011 */
 /* 00001101 */
 /* 00000001 */
 /* 00000001 */
 /* 00000000 */
 /* 'r' */ [0x72] = { 0, { 0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00010110 */
 /* 00011001 */
 /* 00010000 */
 /* 00010000 */
 /* 00010000 */
 /* 00000000 */
 /* 's' */ [0x73] = { 0, { 0x00, 0x00, 0x0f, 0x10, 0x1e, 0x01, 0x1f, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00001111 */
 /* 00010000 */
 /* 00011110 */
 /* 00000001 */
 /* 00011111 */
 /* 00000000 */
 /* 't' */ [0x74] = { 0, { 0x08, 0x08, 0x1c, 0x08, 0x08, 0x09, 0x06, 0x00 }, },
 /* 00001000 */
 /* 00001000 */
 /* 00011100 */
 /* 00001000 */
 /* 00001000 */
 /* 00001001 */
 /* 00000110 */
 /* 00000000 */
 /* 'u' */ [0x75] = { 0, { 0x00, 0x00, 0x12, 0x12, 0x12, 0x12, 0x0d, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00010010 */
 /* 00010010 */
 /* 00010010 */
 /* 00010010 */
 /* 00001101 */
 /* 00000000 */
 /* 'v' */ [0x76] = { 0, { 0x00, 0x00, 0x11, 0x11, 0x11, 0x0a, 0x04, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00010001 */
 /* 00010001 */
 /* 00010001 */
 /* 00001010 */
 /* 00000100 */
 /* 00000000 */
 /* 'w' */ [0x77] = { 0, { 0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0a, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00010001 */
 /* 00010001 */
 /* 00010101 */
 /* 00010101 */
 /* 00001010 */
 /* 00000000 */
 /* 'x' */ [0x78] = { 0, { 0x00, 0x00, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00010001 */
 /* 00001010 */
 /* 00000100 */
 /* 00001010 */
 /* 00010001 */
 /* 00000000 */
 /* 'y' */ [0x79] = { 0, { 0x00, 0x00, 0x11, 0x11, 0x13, 0x0d, 0x01, 0x0e }, },
 /* 00000000 */
 /* 00000000 */
 /* 00010001 */
 /* 00010001 */
 /* 00010011 */
 /* 00001101 */
 /* 00000001 */
 /* 00001110 */
 /* 'z' */ [0x7a] = { 0, { 0x00, 0x00, 0x1f, 0x02, 0x04, 0x08, 0x1f, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00011111 */
 /* 00000010 */
 /* 00000100 */
 /* 00001000 */
 /* 00011111 */
 /* 00000000 */
 /* '{' */ [0x7b] = { 0, { 0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02, 0x00 }, },
 /* 00000010 */
 /* 00000100 */
 /* 00000100 */
 /* 00001000 */
 /* 00000100 */
 /* 00000100 */
 /* 00000010 */
 /* 00000000 */
 /* '|' */ [0x7c] = { 0, { 0x04, 0x04, 0x04, 0x00, 0x04, 0x04, 0x04, 0x00 }, },
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000000 */
 /* 00000100 */
 /* 00000100 */
 /* 00000100 */
 /* 00000000 */
 /* '}' */ [0x7d] = { 0, { 0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08, 0x00 }, },
 /* 00001000 */
 /* 00000100 */
 /* 00000100 */
 /* 00000010 */
 /* 00000100 */
 /* 00000100 */
 /* 00001000 */
 /* 00000000 */
 /* '~' */ [0x7e] = { 0, { 0x08, 0x15, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00001000 */
 /* 00010101 */
 /* 00000010 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x7f] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x80] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x81] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x82] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x83] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x84] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x85] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x86] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x87] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x88] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x89] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x8a] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x8b] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x8c] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x8d] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x8e] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x8f] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x90] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x91] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x92] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x93] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x94] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x95] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x96] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x97] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x98] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x99] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x9a] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x9b] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x9c] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x9d] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x9e] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* '?' */ [0x9f] = { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
 /* 00000000 */
};

int ipcvideo_display_init(struct ipcvideo_display_context *ctx, unsigned int mode)
{
	memset(ctx, 0, sizeof(*ctx));
	
	ctx->plotwidth = 8 * 4; /* The 8bit char is rendered as 32x32 */
	ctx->plotheight = ctx->plotwidth;

	ctx->currx = 0;
	ctx->curry = 0;
	ctx->mode = mode;

	return 0;
}

static int ipcvideo_display_render_moveto(struct ipcvideo_display_context *ctx, int x, int y)
{
	ctx->ptr = ctx->frame + (x * (ctx->plotwidth * 2));

	if (ctx->mode == IPCFOURCC_YUYV)
		ctx->ptr += ((y * (ctx->stride * 2)) * ctx->plotheight);
	else
	if (ctx->mode == IPCFOURCC_BGRX)
		ctx->ptr += ((y * (ctx->stride * 4)) * ctx->plotheight);

	ctx->currx = x;
	ctx->curry = y;

	/* YUY2 colors for a single pixel (background/foreground) */
	ctx->bg[0] = 0x00;
	ctx->bg[1] = 0x80;

	ctx->fg[0] = 0xc0;
	ctx->fg[1] = 0x80;

	return 0;
}

static int ipcvideo_display_render_character_yuy2(struct ipcvideo_display_context *ctx, unsigned char letter)
{
	unsigned char line;

	if (letter > 0x9f)
		return KLAPI_INVALID_ARG;

	for (int i = 0; i < 8; i++) {
	
		int k = 0;
		while (k++ < 4) {
			line = charset[ letter ].data[ i ];
			for (int j = 0; j < 8; j++) {
				if (line & 0x80) {
					/* font color */
					*(ctx->ptr + 0) = ctx->fg[0];
					*(ctx->ptr + 1) = ctx->fg[1];
					*(ctx->ptr + 2) = ctx->fg[0];
					*(ctx->ptr + 3) = ctx->fg[1];
					*(ctx->ptr + 4) = ctx->fg[0];
					*(ctx->ptr + 5) = ctx->fg[1];
					*(ctx->ptr + 6) = ctx->fg[0];
					*(ctx->ptr + 7) = ctx->fg[1];
				} else {
					/* background color */
#if ALPHA_BACKGROUND
					/* Minor alpha */
					*(ctx->ptr + 0) >>= 1;
					*(ctx->ptr + 2) >>= 1;
					*(ctx->ptr + 4) >>= 1;
					*(ctx->ptr + 6) >>= 1;
#else
					/* Complete black background */
					*(ctx->ptr + 0) = ctx->bg[0];
					*(ctx->ptr + 1) = ctx->bg[1];
					*(ctx->ptr + 2) = ctx->bg[0];
					*(ctx->ptr + 3) = ctx->bg[1];
					*(ctx->ptr + 4) = ctx->bg[0];
					*(ctx->ptr + 5) = ctx->bg[1];
					*(ctx->ptr + 6) = ctx->bg[0];
					*(ctx->ptr + 7) = ctx->bg[1];
#endif
				}

				ctx->ptr += 8;
				line <<= 1;
			}
			ctx->ptr += ((ctx->stride * 2) - (8 * 2 * 4));
		}
	}

	return 0;
}

static int ipcvideo_display_render_character_bgrx(struct ipcvideo_display_context *ctx, unsigned char letter)
{
	unsigned char line;

	if (letter > 0x9f)
		return KLAPI_INVALID_ARG;

	/* for each line in the letter */
	for (int i = 0; i < 8; i++) {
	
		int k = 0;
		while (k++ < 4) {
			line = charset[ letter ].data[ i ];

			/* For each pixel on this line.... */
			for (int j = 0; j < 8; j++) {
				if (line & 0x80) {
					/* For each pixel in the line, draw 8 pixels out (enlarge font) */
					for (int z = 0; z < 2; z++) {
						/* font color */
						*(ctx->ptr + (z * 4) + 0) = ctx->fg[0]; // B
						*(ctx->ptr + (z * 4) + 1) = ctx->fg[0]; // G
						*(ctx->ptr + (z * 4) + 2) = ctx->fg[0]; // R
						*(ctx->ptr + (z * 4) + 3) = 0xff;       // X
					}
	
				} else {
					/* background color */
#if ALPHA_BACKGROUND
#else
					/* Complete black background */
					for (int z = 0; z < 2; z++) {
						/* font color */
						*(ctx->ptr + (z * 4) + 0) = ctx->bg[0]; // B
						*(ctx->ptr + (z * 4) + 1) = ctx->bg[0]; // G
						*(ctx->ptr + (z * 4) + 2) = ctx->bg[0]; // R
						*(ctx->ptr + (z * 4) + 3) = 0xff;       // X
					}
#endif
				}

				ctx->ptr += (2 * 4); // 2 x pixels (4 bytes per color)
				line <<= 1;
			}
			ctx->ptr += ((ctx->stride * 4) - (8 * 4 * 2));
		}
	}

	return 0;
}


static int ipcvideo_display_render_ascii(struct ipcvideo_display_context *ctx, unsigned char letter, int x, int y)
{
	if (letter > 0x9f)
		return KLAPI_INVALID_ARG;

	ipcvideo_display_render_moveto(ctx, x, y);

	if (ctx->mode == IPCFOURCC_YUYV)
		ipcvideo_display_render_character_yuy2(ctx, letter);
	else
	if (ctx->mode == IPCFOURCC_BGRX)
		ipcvideo_display_render_character_bgrx(ctx, letter);

	return KLAPI_OK;
}

int ipcvideo_display_render_string(struct ipcvideo_display_context *ctx, unsigned char *s, unsigned int len, unsigned int x, unsigned int y)
{
	if ((!ctx) || (!s) || (len > 80) || (x > 24) || (y > 24))
		return KLAPI_INVALID_ARG;

	for (unsigned i = 0; i < len; i++)
		ipcvideo_display_render_ascii(ctx, *(s + i), x + i, y);

	return KLAPI_OK;
}

int ipcvideo_display_render_reset(struct ipcvideo_display_context *ctx, unsigned char *ptr, unsigned int stride)
{
	unsigned int strides[] = {
		720 * 2, 1280 * 2, 1920 * 2, /* YUYV */
		720 * 4, 1280 * 4, 1920 * 4, /* BGRX */
	};

	if ((!ctx) || (!ptr))
		return KLAPI_INVALID_ARG;

	int found = 0;
	for (unsigned int i = 0; i < (sizeof(strides) / sizeof(unsigned int)); i++) {
		if (strides[i] == stride) {
			found = 1;
			break;
		}
	}
	if (!found)
		return KLAPI_INVALID_ARG;

	ctx->ptr = ptr;
	ctx->frame = ptr;
	if (ctx->mode == IPCFOURCC_YUYV) 
		ctx->stride = stride / 2;
	else
	if (ctx->mode == IPCFOURCC_BGRX)
		ctx->stride = stride / 4;

	ipcvideo_display_render_moveto(ctx, 0, 0);

	return KLAPI_OK;
}

