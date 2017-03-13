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

#endif
