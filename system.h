#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include "config.h"

typedef void (*EventLoopSubscriber)(byte rank);

bool systemInit(void);
bool registerEvent(EventLoopSubscriber subscriber);
bool removeEvent(EventLoopSubscriber subscriber);
void handleEvents();

#endif
