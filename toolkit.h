#ifndef TOOLKIT_H
#define TOOLKIT_H

#include <Arduino.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

char dthchr(byte decimal, bool upperCase);
byte hchrtd(char hexChar);

#endif
