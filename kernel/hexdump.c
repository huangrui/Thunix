/*
 * The hex dump lib.
 *
 * Copyright (C) 2009 Liu Aleaxander -- All rights reserved. This file
 * may be redistributed under the terms of the GNU Public License.
 */

#include <stdio.h>
#include <string.h>

void hexdump(const void *data, int len)
{
	int i = 0;
	int index = 0;
	unsigned int base = 0;
	unsigned char *p = data;
	unsigned char hex[16 * 3 + 1] = {0, };
	unsigned char text[16 + 1] = {0, };
	unsigned char c;

	for (i = 0; i < len; i++) {
		index = i & 0xf;
		if (i && index == 0) {
			/* print the buffer before reset it */
			printk("%08x  %-48s  |%-16s|\n", base, hex, text);
			base += 0x10;
			memset(hex, 0, sizeof hex);
			memset(text, 0, sizeof text);
		}

		c = *p++;
		sprintk((char *)&hex[index * 3], "%02x ", c);
		if (c < 0x20 || c > 0x7f)
			text[index] = '.';
		else
			text[index] = c;
	}

	/* print the last part */
	printk("%08x  %-48s  |%-16s|\n", base, hex, text);
}
