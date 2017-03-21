#ifndef TIMEWATCH_H
#define TIMEWATCH_H

#include <Arduino.h>
#include <TimeLib.h>
#include "config.h"
#include "textView.h"
#include "keyUtils.h"
#include "userInterface.h"

#define TIME_TO_SECS(h, m, s) (h * 3600 + m * 60 + s)
#define CUR_TIME_TO_SECS() TIME_TO_SECS(hour(now()), minute(now()), second(now()))
#define CIRCLE_X(val, radius) round( cos((PI/180)*(double)val) * radius)
#define CIRCLE_Y(val, radius) round( sin((PI/180)*(double)val) * radius)
//#define CIRCLE_X(val, max, rotation, radius) round( cos( ((2*PI)/(double)max)*(double)val + rotation) * radius)
//#define CIRCLE_Y(val, max, rotation, radius) round( sin( ((2*PI)/(double)max)*(double)val + rotation) * radius)

#endif
