#ifndef BITOPTS_H
#define BITOPTS_H

extern int set_bit(void *, unsigned int);
extern int clear_bit(void *, unsigned int);
extern int test_bit(const void *, unsigned int);
extern unsigned long find_first_zero(void *, void *);


#endif /* bitopts.h */
