#ifndef LISTTOOLS_H
#define LISTTOOLS_H

#include <Arduino.h>
#include "config.h"
#include "keyUtils.h"

const char* mergeItemPtr(byte index, const char** items1, byte itemCount1, const char** items2, byte itemCount2);
byte findClosestIndices(char* keyString, const char** items, byte itemCount, byte* indices, byte maxIndices);
byte findClosestIndicesFiltered(char* keyString, const char** items, byte itemCount, byte* blacklist, byte blacklistCount, byte* indices, byte maxIndices);
byte findClosestIndicesDual(char* keyString, const char** items1, byte itemCount1, const char** items2, byte itemCount2, byte* indices, byte maxIndices, byte* blacklist, byte blacklistCount);
byte findClosestIndicesDualFiltered(char* keyString, const char** items1, byte itemCount1, const char** items2, byte itemCount2, byte* blacklist, byte blacklistCount, byte* indices, byte maxIndices);

#endif
