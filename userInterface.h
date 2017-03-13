#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>
#include "RPN-Calc.h"
#include "keyUtils.h"
#include "textView.h"
#include "listTools.h"

void fadeLedTo(byte pin, byte fromBright, byte toBright, unsigned int duration);
void fadeLed(byte pin, bool endState, unsigned int duration, byte maxBright);
byte uiMessagePrintf(const char* format, const char* buttons, ...);
bool doubleEntry(double* value, byte digits);
bool intEntry(int* value, bool allowNegative);
byte showItemMenu(char* title, byte defaultItem, const char** allItems, byte numItems, byte* shownItems, byte numShownItems);
byte showDualItemMenu(char* title, byte defaultItem, const char** allItems1, byte numItems1, const char** allItems2, byte numItems2, byte* shownItems, byte numShownItems);
bool smartMenu(const char* title, const char** items, byte itemCount, byte maxItemsListed, byte* selection);
bool smartFilteredMenu(const char* title, const char** items, byte itemCount, byte* blacklist, byte blacklistCount, byte maxItemsListed, byte* selection);
bool smartDualFilteredMenu(const char* title, const char** items1, byte itemCount1, const char** items2, byte itemCount2, byte* blacklist, byte blacklistCount, byte maxItemsListed, byte* selection);

#endif
