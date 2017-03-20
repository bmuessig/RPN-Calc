#ifndef RPN_CALC_H
#define RPN_CALC_H

#include <Arduino.h>
#include <TimeLib.h>

#include "config.h"
#include "system.h"
#include "settings.h"
#include "hardware.h"
#include "textView.h"
#include "keyUtils.h"
#include "userInterface.h"
#include "schedule.h"
#include "headerUtils.h"

#include "stackMathGlob.h"
#include "timeWatchGlob.h"
#include "snakeGlob.h"
#include "settingsViewGlob.h"

void setup(void);
void loop(void);
void doEvents(byte rank);

#endif
