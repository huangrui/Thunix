#ifndef BITOPTS_H
#define BITOPTS_H

/*
 * Set the bit and return it's original value
 */
static int set_bit(void * addr, unsigned int nr)
{
	int		mask, retval;
	unsigned char	*ADDR = (unsigned char *) addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	retval = mask & *ADDR;
	*ADDR |= mask;
	return retval;
}

/* 
 * Clear the bit and return it's original value
 */
static int clear_bit(void * addr, unsigned int nr)
{
	int		mask, retval;
	unsigned char	*ADDR = (unsigned char *) addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	retval = mask & *ADDR;
	*ADDR &= ~mask;
	return retval;
}

static int test_bit(const void * addr, unsigned int nr)
{
	int			mask;
	const unsigned char	*ADDR = (const unsigned char *) addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	return (mask & *ADDR);
}

static unsigned long find_first_zero(void *buf, void *end)
{
	unsigned long *p = (unsigned long *)buf;
	unsigned long block = 0;
	int i;

	while (*p == 0xffffffff && (void *)p < end) {
		p++;
		block += 32;
	}
	if ((void *)p >= end)
		return -1;

	for (i = 0; i < 32; i++)
		if (test_bit(p, i) == 0)
			return block + i;
}


#endif /* bitopts.h */
