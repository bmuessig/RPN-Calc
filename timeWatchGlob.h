#ifndef TIMEWATCHGLOB_H
#define TIMEWATCHGLOB_H
#include <Arduino.h>
#include "textView.h"

void timeWatch(void);
bool timeWatchRenderClock(GraphicsArea *renderArea, bool invertColors, byte hour, byte minute, byte second);
bool timeWatchTimerStart(byte hour, byte minute);
void timeWatchTimerStop(void);
bool timeWatchTimerRunning(void);
bool timeWatchTimerStatus(char* statusBuffer);

#endif
