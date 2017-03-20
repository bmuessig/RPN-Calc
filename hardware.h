#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SparkFunMAX17043.h>
#include <Keypad.h>
#include <TimeLib.h>
#include "keyUtils.h"

#define lcdWidth 84
#define lcdHeight 48

extern const byte lcdDC, lcdCS, lcdRS, lcdBL;
extern byte lcdContrast, lcdBrightness;
extern U8G2_PCD8544_84X48_F_4W_HW_SPI display;

extern const byte keypadRows[4];
extern const byte keypadCols[3];
extern Keypad keypad;

extern const byte led;
extern byte ledBrightness;

extern File sdRoot;
extern const byte sdCS;

#define CPU_RESTART_ADDR (uint32_t*)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART() (*CPU_RESTART_ADDR = CPU_RESTART_VAL)

void hwDeepSleep(bool enable);

#endif
