#ifndef KEYUTILS_H
#define KEYUTILS_H

#include <Arduino.h>
#include "system.h"
#include "hardware.h"

extern const char keymap[4][3];
extern const byte charMapSegments, charMapFields;
extern const char charMap[];

void uiControl(void);
void keyControl(void);
char awaitKey(void);
bool calmKeys(unsigned short cooldown, unsigned short timeout);
bool matchMapSegment(char key, byte* segment);
bool mapSegmentMatchesChar(char chr, byte segment);

#endif
