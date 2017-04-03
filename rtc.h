#ifndef RTC_H
#define RTC_H

#include <TimeLib.h>
#include <Timezone.h>

void rtcInit(void);
void rtcSet(time_t utcTime);
time_t rtcGet(void);
void rtcReset(void);

time_t compileTime(void);

#endif
