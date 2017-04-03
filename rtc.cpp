#include "rtc.h"

//Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone CE(CEST, CET);

void rtcInit(void) {
  setSyncProvider((getExternalTime)Teensy3Clock.get);
}

void rtcSet(time_t utcTime) {
  setTime(utcTime);
}

void rtcReset(void) {
  setTime(CE.toUTC(compileTime())); //sets the time to the compile time
}

time_t rtcGet(void) {
  TimeChangeRule *tcr;
  return CE.toLocal(now(), &tcr);
}

//Function to return the compile date and time as a time_t value
time_t compileTime(void) {
    const int offset = 25;        //fudge factor to allow for compile time (seconds, YMMV)
    char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char chMon[4], *m;
    int d, y;
    tmElements_t tm;
    time_t t;

    strncpy(chMon, compDate, 3);
    chMon[3] = '\0';
    m = strstr(months, chMon);
    tm.Month = ((m - months) / 3 + 1);

    tm.Day = atoi(compDate + 4);
    tm.Year = atoi(compDate + 7) - 1970;
    tm.Hour = atoi(compTime);
    tm.Minute = atoi(compTime + 3);
    tm.Second = atoi(compTime + 6);
    t = makeTime(tm);
    return t + offset;        //add fudge factor to allow for compile time
}
