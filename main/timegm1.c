#include "timegm1.h"
#include "time.h"

/*
 * UTC version of mktime(3)
 */

/*
 * This code is not portable, but works on most Unix-like systems.
 * If the local timezone has no summer time, using mktime(3) function
 * and adjusting offset would be usable (adjusting leap seconds
 * is still required, though), but the assumption is not always true.
 *
 * Anyway, no portable and correct implementation of UTC to time_t
 * conversion exists....
 */

#define EPOCH_YEAR		1970
#define TM_YEAR_BASE	1900

static time_t
sub_mkgmt(struct tm *tm)
{
	int y, nleapdays;
	time_t t;
	/* days before the month */
	static const unsigned short moff[12] = {
		0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
	};

	/*
	 * XXX: This code assumes the given time to be normalized.
	 * Normalizing here is impossible in case the given time is a leap
	 * second but the local time library is ignorant of leap seconds.
	 */

	/* minimal sanity checking not to access outside of the array */
	if ((unsigned) tm->tm_mon >= 12)
		return (time_t) -1;
	if (tm->tm_year < EPOCH_YEAR - TM_YEAR_BASE)
		return (time_t) -1;

	y = tm->tm_year + TM_YEAR_BASE - (tm->tm_mon < 2);
	nleapdays = y / 4 - y / 100 + y / 400 -
	    ((EPOCH_YEAR-1) / 4 - (EPOCH_YEAR-1) / 100 + (EPOCH_YEAR-1) / 400);
	t = ((((time_t) (tm->tm_year - (EPOCH_YEAR - TM_YEAR_BASE)) * 365 +
			moff[tm->tm_mon] + tm->tm_mday - 1 + nleapdays) * 24 +
		tm->tm_hour) * 60 + tm->tm_min) * 60 + tm->tm_sec;

	return (t < 0 ? (time_t) -1 : t);
}

time_t timegm_1(struct tm *tm)
{
	static const unsigned int ndays[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
  };
  time_t r = 0;
  int i;
  unsigned int *nday = (unsigned int*) ndays[lpyear(tm->tm_year+1900)];

  static const int epoch_year = 70;
  if(tm->tm_year >= epoch_year) {
    for (i = epoch_year; i < tm->tm_year; ++i)
      r += lpyear(i+1900) ? 366*24*60*60 : 365*24*60*60;
  } else {
    for (i = tm->tm_year; i < epoch_year; ++i)
      r -= lpyear(i+1900) ? 366*24*60*60 : 365*24*60*60;
  }
  for (i = 0; i < tm->tm_mon; ++i)
    r += nday[i] * 24 * 60 * 60;
  r += (tm->tm_mday - 1) * 24 * 60 * 60;
  r += tm->tm_hour * 60 * 60;
  r += tm->tm_min * 60;
  r += tm->tm_sec;
  return r;


/*    const int SecondsPerMinute = 60;
  const int SecondsPerHour = 3600;
  const int SecondsPerDay = 86400;
  const int DaysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  time_t secs = 0;
  // tm_year is years since 1900
  int year = ptm->tm_year + 1900;
  for (int y = 1970; y < year; ++y) {
    secs += (IsLeapYear(y) ? 366 : 365) * SecondsPerDay;
  }
  // tm_mon is month from 0..11
  for (int m = 0; m < ptm->tm_mon; ++m) {
    secs += DaysOfMonth[m] * SecondsPerDay;
    if (m == 1 && IsLeapYear(year))
      secs += SecondsPerDay;
  }
  secs += (ptm->tm_mday - 1) * SecondsPerDay;
  secs += ptm->tm_hour * SecondsPerHour;
  secs += ptm->tm_min * SecondsPerMinute;
  secs += ptm->tm_sec;
  return secs; */
}

static unsigned int lpyear(unsigned int y)
{
  return (y % 4) == 0 && ((y % 100) != 0 || (y % 400) == 0);
}
