#ifndef __TIMEGM__H
#define __TIMEGM__H

#ifndef HAVE_TIMEGM

#include <time.h>

time_t timegm_1(struct tm *tm);
static unsigned int lpyear(unsigned int y);
#endif

#endif
