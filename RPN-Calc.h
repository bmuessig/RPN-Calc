#ifndef RPN_CALC_H
#define RPN_CALC_H

#include <Arduino.h>
#include <TimeLib.h>

#include "hardware.h"
#include "textView.h"
#include "keyUtils.h"
#include "userInterface.h"
#include "schedule.h"

#include "stackMathGlob.h"
#include "timeWatchGlob.h"
#include "snakeGlob.h"
#include "settingsViewGlob.h"

#define VERSION "2.0"
#define CLOCKSPEED 72
#define PROCESSOR "NXP MK20DX256"
#define MEMORY 64

//#define DEBUG

#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART() (*CPU_RESTART_ADDR = CPU_RESTART_VAL)

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

void setup(void);
void loop(void);
void doEvents(void);

#endif
