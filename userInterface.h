#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>
#include "RPN-Calc.h"
#include "keyUtils.h"
#include "textView.h"

void fadeLedTo(byte pin, byte fromBright, byte toBright, unsigned int duration);
void fadeLed(byte pin, bool endState, unsigned int duration, byte maxBright);
byte uiMessagePrintf(const char* format, const char* buttons, ...);
bool doubleEntry(double* value, byte digits);
bool intEntry(int* value, bool allowNegative);
byte findClosestIndices(char* keyString, const char** items, byte itemCount, byte* indices, byte maxIndices);
byte showItemMenu(char* title, byte defaultItem, const char** allItems, byte numItems, byte* shownItems);
bool smartMenu(const char* title, const char** items, byte itemCount, byte maxItemsListed, byte* selection);

#endif
