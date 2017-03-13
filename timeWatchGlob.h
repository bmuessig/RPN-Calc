#ifndef TIMEWATCHGLOB_H
#define TIMEWATCHGLOB_H

void timeWatch(void);
bool timeWatchTimerStart(byte hour, byte minute);
void timeWatchTimerStop(void);
bool timeWatchTimerRunning(void);
bool timeWatchTimerStatus(char* statusBuffer);

#endif
