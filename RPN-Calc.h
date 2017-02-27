#ifndef RPN_CALC_H
#define RPN_CALC_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SparkFunMAX17043.h>
#include <Keypad.h>

#define VERSION "1.5"
//#define DEBUG

#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART() (*CPU_RESTART_ADDR = CPU_RESTART_VAL)

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

extern const byte lcdDC, lcdCS, lcdRS, lcdBL;
extern byte lcdContrast, lcdBrightness;
extern U8G2_PCD8544_84X48_F_4W_HW_SPI display;

extern unsigned long lastEventQuery;

extern const char keymap[4][3];
extern const byte keypadRows[4];
extern const byte keypadCols[3];
extern Keypad keypad;

extern const byte led;
extern byte ledBrightness;

extern File sdRoot;
extern const byte sdCS;

void doEvents(void);

#endif
