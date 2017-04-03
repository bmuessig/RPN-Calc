#include "toolkit.h"

char dthchr(byte decimal, bool upperCase) {
  decimal %= 0xF; // Clamp digit to be between 0 and 9
  if(decimal < 10)
    return '0' + decimal;
  if(upperCase)
    return 'A' + decimal - 10;
  return 'a' + decimal - 10;
}

byte hchrtd(char hexChar) {
  if(hexChar >= 'A' && hexChar <= 'F') // uppercase
    return hexChar - 'A';
  else if(hexChar >= 'a' && hexChar <= 'f')
    return hexChar - 'a';
  else if(hexChar >= '0' && hexChar <= '9')
    return hexChar - '0';
  return 0xFF; // error
}
