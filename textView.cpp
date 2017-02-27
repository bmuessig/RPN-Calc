#include <Arduino.h>
#include "RPNCalc.h"
#include "textView.h"

const byte textViewChrW = 4, textViewChrH = 6, textViewRows = 8, textViewCols = 21, textViewStatusLen = 10,
  textViewTitleLen = textViewCols - textViewStatusLen;
char textViewTitle[textViewTitleLen + 1] = {0};
byte textViewRow = 0, textViewCol = 0, textViewColor = 0;
bool textViewImmLf = false, textViewNextLf = false, textViewFullscreen = true, textViewAllowAutoRender = false;
unsigned short textView[textViewRows * textViewCols] = { 32 };

void textViewPutStrAt(String str, byte color, byte row, byte col) {
  byte oldRow = textViewRow, oldCol = textViewCol, oldColor = textViewColor;
  bool oldNextLf = textViewNextLf;
  textViewRow = row;
  textViewCol = col;
  textViewColor = color;
  textViewNextLf = false;
  textViewPutStr(str);
  textViewRow = oldRow;
  textViewCol = oldCol;
  textViewColor = oldColor;
  textViewNextLf = oldNextLf;
}

void textViewPutCStrAt(char* str, byte color, byte row, byte col) {
  byte oldRow = textViewRow, oldCol = textViewCol, oldColor = textViewColor;
  bool oldNextLf = textViewNextLf;
  textViewRow = row;
  textViewCol = col;
  textViewColor = color;
  textViewNextLf = false;
  textViewPutCStr(str);
  textViewRow = oldRow;
  textViewCol = oldCol;
  textViewColor = oldColor;
  textViewNextLf = oldNextLf;
}

bool textViewGoto(int row, int col) {
  if(row >= textViewRows || col >= textViewCols)
    return false;
  if(row >= 0)
    textViewRow = row;
  if(col >= 0)
    textViewCol = col;
  return true;
}

bool textViewSet(int row, int col, byte color) {
  if(!textViewGoto(row, col))
    return false;
  if(color > 3)
    return false;
  textViewColor = color;
  return true;
}

bool textViewErase(byte row, byte colStart, byte colEnd) {
  if(colEnd >= textViewCols || row >= textViewRows)
    return false;
  for(; colStart < colEnd; colStart++)
    textView[row * textViewCols + colStart] = 32 << 2;
  return true;
}

bool textViewEraseLine(byte row) {
  return textViewErase(row, 0, textViewCols - 1);
}

void textViewPutChrAt(char chr, byte color, byte row, byte col) {
  byte oldRow = textViewRow, oldCol = textViewCol, oldColor = textViewColor;
  bool oldNextLf = textViewNextLf;
  textViewRow = row;
  textViewCol = col;
  textViewColor = color;
  textViewNextLf = false;
  textViewPutChr(chr);
  textViewRow = oldRow;
  textViewCol = oldCol;
  textViewColor = oldColor;
  textViewNextLf = oldNextLf;
}

void textViewPutStr(String str) {
  char sbuf[str.length() + 1];
  str.toCharArray(sbuf, str.length() + 1);
  textViewPutCStr(sbuf);
}

void textViewPutCStr(char* str) {
  for(; *(str); str++)
    textViewPutChr(*(str));
}

unsigned int textViewPrintf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  unsigned int strLength;
  if(strLength = vsnprintf(NULL, 0, format, args)) {
    va_end(args);
    char sbuf[strLength + 2];
    va_start(args, format);
    vsnprintf(sbuf, strLength + 1, format, args);
    textViewPutCStr(sbuf);
  }

  va_end(args);
  return strLength;
}

unsigned int textViewPrintfAt(const char* format, byte color, byte row, byte col, ...) {
  byte oldRow = textViewRow, oldCol = textViewCol, oldColor = textViewColor;
  bool oldNextLf = textViewNextLf;
  textViewRow = row;
  textViewCol = col;
  textViewColor = color;
  textViewNextLf = false;
  
  va_list args;
  va_start(args, col);
  unsigned int strLength;
  if(strLength = vsnprintf(NULL, 0, format, args)) {
    va_end(args);
    char sbuf[strLength + 2];
    va_start(args, col);
    vsnprintf(sbuf, strLength + 1, format, args);
    textViewPutCStr(sbuf);
  }
  va_end(args);

  textViewRow = oldRow;
  textViewCol = oldCol;
  textViewColor = oldColor;
  textViewNextLf = oldNextLf;
  return strLength;
}

unsigned int textViewNPrintfAt(const char* format, unsigned int maxLength, byte color, byte row, byte col, ...) {
  byte oldRow = textViewRow, oldCol = textViewCol, oldColor = textViewColor;
  bool oldNextLf = textViewNextLf;
  char sbuf[maxLength + 2];
  textViewRow = row;
  textViewCol = col;
  textViewColor = color;
  textViewNextLf = false;
  
  va_list args;
  va_start(args, col);
  unsigned int outLength = vsnprintf(sbuf, maxLength + 1, format, args);
  textViewPutCStr(sbuf);
  va_end(args);

  textViewRow = oldRow;
  textViewCol = oldCol;
  textViewColor = oldColor;
  textViewNextLf = oldNextLf;
  return outLength;
}

unsigned int textViewNPrintf(const char* format, unsigned int maxLength, ...) {
  char sbuf[maxLength + 2];
  va_list args;
  va_start(args, maxLength);
  unsigned int outLength = vsnprintf(sbuf, maxLength + 1, format, args);
  textViewPutCStr(sbuf);
  va_end(args);
  return outLength;
}

void textViewLinefeed(void) {
  if(textViewRow == textViewRows - 1) {
    for(byte row = (textViewFullscreen ? 1 : 2); row < textViewRows; row++) {
      for(byte col = 0; col < textViewCols; col++)
        textView[(row - 1) * textViewCols + col] = textView[row * textViewCols + col];
    }
    
    for(byte col = 0; col < textViewCols; col++)
      textView[(textViewRows - 1) * textViewCols + col] = 32 << 2;
  } else
    textViewRow++;
  textViewCol = 0;
  textViewNextLf = false;
}

void textViewBackspace(void) {
  // TODO: implement backspace
}

void textViewClear(void) {
  for(unsigned int i = 0; i < textViewRows * textViewCols; i++)
    textView[i] = 32;
  textViewRow = textViewFullscreen ? 0 : 1;
  textViewCol = 0;
  textViewNextLf = false;
  textViewColor = 0;
}

// Used after exiting a sub program
void textViewClearAll(void) {
  textViewFullscreen = true;
  textViewAllowAutoRender = true;
  textViewEraseTitle();
  textViewClear();
  if(!textViewFullscreen)
    textViewClearStatus();
}

bool textViewClearStatus(void) {
  if(textViewFullscreen)
    return false;
  for(unsigned int i = 0; i < textViewCols; i++)
      textViewPutChrAt(' ', 0, 0, i);
   return true;
}

bool textViewSetStatus(char* str, byte col) {
  if(textViewFullscreen)
    return false;
  if(textlen(str) > textViewCols - col)
    return false;
  textViewPutCStrAt(str, 1, 0, col);
  return true;
}

bool textViewSetCTitle(char* title) {
  if(textViewFullscreen)
    return false;
  if(strlen(title) > textViewTitleLen)
    return false;
  for(byte i = 0; i <= textViewTitleLen; i++)
    textViewTitle[i] = 0;
  strncpy(textViewTitle, title, textViewTitleLen);
  return true;
}

bool textViewSetTitle(String title) {
  if(textViewFullscreen)
    return false;
  if(title.length() > textViewTitleLen)
    return false;
  for(byte i = 0; i <= textViewTitleLen; i++)
    textViewTitle[i] = 0;
  title.toCharArray(textViewTitle, textViewTitleLen + 1);
  return true;
}

void textViewEraseTitle(void) {
  memset(textViewTitle, 0, textViewTitleLen);
}

bool textViewStatusUpdate(void) {
  if(textViewFullscreen)
    return false;
  char str[13] = {0}, batt = map(lipo.getSOC(), 0, 100, '0', '9');
  sprintf(str, "\a \a[%c] %02d.%02d", batt, hour(now()), minute(now()));
  textViewClearStatus();
  return (textViewSetStatus(textViewTitle, 0) && textViewSetStatus(str, textViewTitleLen));
}

bool textViewPutChr(char chr) {
  if(textViewNextLf)
    textViewLinefeed();
  if(chr == '\n') {
    textViewLinefeed();
    return true;
  } else if(chr == '\b') {
    textViewBackspace();
    return true;
  } else if(chr == '\a') {
    if(textViewColor == 0)
      textViewColor = 1;
    else if(textViewColor == 1)
      textViewColor = 0;
    return true;
  } else if(chr >= 32) {
    textView[(unsigned int)textViewRow * textViewCols + textViewCol] = (chr << 2) | (textViewColor & 0x3);
    
    if(textViewCol + 1 >= textViewCols)
      if(textViewImmLf)
        textViewLinefeed();
      else
        textViewNextLf = true;
    else
      textViewCol++;
    return true;
  }

  return false;
}

void textViewRender(void) {
  display.setCursor(0,0);
  display.clearBuffer();
  textViewDraw();
  display.sendBuffer();
}

void textViewDraw(void) {
  // set the font
  display.setFont(textViewFont);
  
  for(byte row = 0; row < textViewRows; row++) {
    for(byte col = 0; col < textViewCols; col++) {
      unsigned short glyph = textView[row * textViewCols + col];
      // Determine the style
      switch(glyph & 0x3) {
        case TV_COLOR_F1H0:
          // Solid background
          display.setFontMode(0);
          // Foreground black
          display.setDrawColor(1);
          break;
        case TV_COLOR_F0H1:
          // Solid background
          display.setFontMode(0);
          // Foreground white
          display.setDrawColor(0);
          break;
        case TV_COLOR_F1HT:
          // Transparent background
          display.setFontMode(1);
          // Foreground black
          display.setDrawColor(1);
          break;
        case TV_COLOR_FXHT:
          // Transparent background
          display.setFontMode(1);
          // Foreground XOR
          display.setDrawColor(2);
          break;
      }
      
      display.drawGlyph(col * textViewChrW, (row + 1) * textViewChrH, textView[row * textViewCols + col] >> 2);
    }
  }
}

unsigned int textlen(char* str) {
  unsigned int len = 0;
  while(*str) {
    if(*str >= 32)
      len++;
    str++;
  }

  return len;
}

