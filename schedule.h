#ifndef SCHEDULE_H
#define SCHEDULE_H

typedef struct {
  unsigned int updated;
  unsigned int tick;
  unsigned int delta;
  void (*event)(void);
} Schedule;

void scheduleInit(unsigned int tick, void (*event)(void), Schedule* schedule);
bool scheduleRun(Schedule* schedule);

#endif
