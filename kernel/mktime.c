#include <time.h>


#define MINUTE 60
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define YEAR (365*DAY)

/* interestingly, we assume leap-years */
static int month[12] = {
	0,
	DAY*(31),
	DAY*(31+28),
	DAY*(31+28+31),
	DAY*(31+28+31+30),
	DAY*(31+28+31+30+31),
	DAY*(31+28+31+30+31+30),
	DAY*(31+28+31+30+31+30+31),
	DAY*(31+28+31+30+31+30+31+31),
	DAY*(31+28+31+30+31+30+31+31+30),
	DAY*(31+28+31+30+31+30+31+31+30+31),
	DAY*(31+28+31+30+31+30+31+31+30+31+30)
};

long kernel_mktime(struct tm * tm)
{
	long res;
	int year;
	int leap;

	year = tm->tm_year - 1;
	leap = ((tm->tm_year % 4)?0:((tm->tm_year % 100)?0:1));

	res = YEAR*year + DAY*(year/4);
	res += month[tm->tm_mon];

	if (tm->tm_mon>1 && leap)
		res += DAY;
	res += DAY*(tm->tm_mday-1);
	res += HOUR*tm->tm_hour;
	res += MINUTE*tm->tm_min;
	res += tm->tm_sec;
	return res;
}
