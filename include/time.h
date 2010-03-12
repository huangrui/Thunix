#ifndef TIME_H
#define TIME_H


#ifndef _TIME_T
#define _TIME_T
typedef long time_t;
#endif

/*#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif*/

extern long startup_time;

#define CLOCKS_PER_SEC 100

typedef long clock_t;

struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
};

extern long kernel_mktime(struct tm*);


#endif  /* time.h */
