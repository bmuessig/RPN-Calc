#include <Arduino.h>
#include "schedule.h"

void scheduleInit(unsigned int tick, void (*event)(void), Schedule* schedule) {
  schedule->tick = tick;
  schedule->event = event;
  schedule->updated = 0;
  schedule->delta = 0;
  schedule->skip = 0;
}

bool scheduleRun(Schedule* schedule) {
  schedule->delta += millis() - schedule->updated;
  schedule->updated = millis();

  if(schedule->delta >= schedule->tick) {
    unsigned int remainder = schedule->delta % schedule->tick;
    schedule->delta -= remainder;
    schedule->skip += schedule->delta / schedule->tick - 1; // how many cycles are skipped
    schedule->delta = remainder;
    if(schedule->event != NULL)
      schedule->event();
    return true;
  }

  return false;
}
