#include "textView.h"
#include "textViewPrivate.h"

// Copyright 2017 Benedikt Muessig <mail@bmuessig.eu>
// All rights reserved
// DO NOT DISTRIBUTE COPIES OF THIS SOFTWARE
// DO NOT REMOVE THIS HEADER

const byte textViewChrW = 4, textViewChrH = 6,
  textViewRows = lcdHeight / textViewChrH /*8*/,
  textViewCols = lcdWidth / textViewChrW /*21*/,
  textViewStatusLen = 10, textViewTitleLen = textViewCols - textViewStatusLen;
char textViewTitle[textViewTitleLen + 1];
byte textViewRow = 0, textViewCol = 0, textViewColor = 0;
bool textViewImmLf = false, textViewNextLf = false, textViewFullscreen = true, textViewAllowAutoRender = false;
unsigned short textView[textViewRows * textViewCols];
StatusUpdater textViewStatusUpdater;

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

void textViewPutCCStrAt(const char* str, byte color, byte row, byte col) {
  byte oldRow = textViewRow, oldCol = textViewCol, oldColor = textViewColor;
  bool oldNextLf = textViewNextLf;
  textViewRow = row;
  textViewCol = col;
  textViewColor = color;
  textViewNextLf = false;
  textViewPutCCStr(str);
  textViewRow = oldRow;
  textViewCol = oldCol;
  textViewColor = oldColor;
  textViewNextLf = oldNextLf;
}

bool textViewGoto(byte row, byte col) {
  if(row >= textViewRows || col >= textViewCols)
    return false;
  if(row >= 0)
    textViewRow = row;
  if(col >= 0)
    textViewCol = col;
  return true;
}

// Allocate graphics buffer within the text buffer
bool textViewGraphics(byte originRow, byte originCol, byte rows, byte cols,
  byte color, GraphicsArea* area) {
  color &= 0x7;
  if(originRow + rows >= textViewRows || originCol + cols >= textViewCols
    || !rows || !cols || !area
    || (color != TV_COLOR_F1HT && color != TV_COLOR_F0HT && color != TV_COLOR_FXHT))
    return false;
  area->originX = originCol * textViewChrW;
  area->originY = originRow * textViewChrH;
  area->width = cols * textViewChrW;
  area->height = rows * textViewChrH;
#ifdef DEBUG
  Serial.print("OriginRow: ");
  Serial.println(originRow, DEC);
  Serial.print("OriginCol: ");
  Serial.println(originCol, DEC);
  Serial.print("Rows: ");
  Serial.println(rows, DEC);
  Serial.print("Cols: ");
  Serial.println(cols, DEC);
  Serial.print("OriginX: ");
  Serial.println(area->originX, DEC);
  Serial.print("OriginY: ");
  Serial.println(area->originY, DEC);
  Serial.print("Width: ");
  Serial.println(area->width, DEC);
  Serial.print("Height: ");
  Serial.println(area->height, DEC);
#endif
  return textViewPaint(originRow, originCol, rows, cols, color);
}

void textViewWipeGraphics(void) {
  display.clearBuffer();
}

bool textViewSeek(signed char offset) {
  return textViewSeekFill(offset, 0, 0);
}

bool textViewSeekFill(signed char offset, char character, byte color) {
  if((character && character < 32) || (color &= 0x7) > 4)
    return false;
  if(offset >= 0) {
#ifdef DEBUG
    Serial.print("Positive offset: ");
    Serial.println(offset, DEC);
#endif
    for(; offset > 0; offset--) {
      if(character)
        textView[textViewRow * textViewCols + textViewCol] = (character << 3) | color;
      if(textViewCol + 1 < textViewCols)
        textViewCol++;
      else
        textViewLinefeed();
    }
  } else {
#ifdef DEBUG
    Serial.print("Negative offset: ");
    Serial.println(offset, DEC);
#endif
    for(; offset < 0; offset++) {
      if(character)
        textView[textViewRow * textViewCols + textViewCol] = (character << 3) | color;
      if(textViewCol)
        textViewCol--;
      else {
        if(textViewRow) {
          textViewRow--;
          textViewCol = 0;
        } else {
          Serial.println("Fail");
          return false; // out of bounds
        }
      }
    }
  }
  return true;
}

bool textViewPaint(byte row, byte col, byte rows, byte cols, byte color) {
  return textViewFill(row, col, rows, cols, 0, color);
}

bool textViewFill(byte row, byte col, byte rows, byte cols, char character, byte color) {
  if(row + rows > textViewRows || col + cols > textViewCols || (color &= 0x7) > 4
    || (character && character < 32))
    return false;
  if(!character)
    character = 32;
  byte startCol = col;
  rows += row;
  cols += col;
  for(; row < rows; row++) {
    for(col = startCol; col < cols; col++) {
      if(character)
        textView[row * textViewCols + col] = (character << 3) | color;
      else {
        textView[row * textViewCols + col] &= (~0x7); // clear the last 3 bits
        textView[row * textViewCols + col] |= color; // bitmask the color
      }
    }
  }
  return true;
}

bool textViewSet(byte row, byte col, byte color) {
  if(!textViewGoto(row, col))
    return false;
  if(color > 3)
    return false;
  textViewColor = color;
  return true;
}

bool textViewSetColor(byte color) {
  if(color > 3)
    return false;
  textViewColor = color;
  return true;
}

void textViewSetImmLf(bool immLf) {
  textViewImmLf = immLf;
}

bool textViewIsFullscreen(void) {
  return textViewFullscreen;
}

void textViewSetFullscreen(bool fullscreen) {
  textViewFullscreen = fullscreen;
}

void textViewSetAllowAutoRender(bool allowAutoRender) {
  textViewAllowAutoRender = allowAutoRender;
}

bool textViewErase(byte row, byte colStart, byte colEnd) {
  if(colEnd >= textViewCols || row >= textViewRows)
    return false;
  for(; colStart <= colEnd; colStart++)
    textView[row * textViewCols + colStart] = (32 << 3) | TV_COLOR_F1H0;
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

void textViewPutCCStr(const char* str) {
  for(; *(str); str++)
    textViewPutChr(*(str));
}

unsigned int textViewPrintf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  unsigned int strLength;
  if((strLength = vsnprintf(NULL, 0, format, args))) {
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
  if((strLength = vsnprintf(NULL, 0, format, args))) {
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
    textViewEraseLine(textViewRows - 1);
  } else
    textViewRow++;
  textViewCol = 0;
  textViewNextLf = false;
}

void textViewBackspace(void) {
  textViewSeekFill(-1, 32, TV_COLOR_F1H0);
}

// Clear the entire screen
void textViewClear(void) {
  for(byte line = 0; line < textViewRows; line++)
    textViewEraseLine(line);
  textViewRow = textViewFullscreen ? 0 : 1;
  textViewCol = 0;
  textViewNextLf = false;
  textViewColor = 0;
}

// Used after exiting a sub program
void textViewClearAll(void) {
  textViewFullscreen = false;
  textViewAllowAutoRender = true;
  textViewEraseTitle();
  textViewWipeGraphics();
  textViewClear();
  textViewClearStatus();
}

bool textViewClearStatus(void) {
  if(textViewFullscreen)
    return false;
  textViewEraseLine(0);
  return true;
}

bool textViewPutStatus(char* str, byte col) {
  if(textViewFullscreen)
    return false;
  if(textlen(str) > (unsigned int)(textViewCols - col))
    return false;
  textViewPutCStrAt(str, 1, 0, col);
  return true;
}

bool textViewSetCTitle(char* title) {
  if(textViewFullscreen)
    return false;
  if(strlen(title) > textViewTitleLen)
    return false;
  textViewEraseTitle();
  strncpy(textViewTitle, title, textViewTitleLen);
  return true;
}

bool textViewSetCCTitle(const char* title) {
  if(textViewFullscreen)
    return false;
  if(strlen(title) > textViewTitleLen)
    return false;
  textViewEraseTitle();
  strncpy(textViewTitle, title, textViewTitleLen);
  return true;
}

bool textViewSetTitle(String title) {
  if(textViewFullscreen)
    return false;
  if(title.length() > textViewTitleLen)
    return false;
  textViewEraseTitle();
  title.toCharArray(textViewTitle, textViewTitleLen + 1);
  return true;
}

void textViewEraseTitle(void) {
  memset(textViewTitle, 0, textViewTitleLen);
}

bool textViewStatusRegister(StatusUpdater updater) {
  if(!updater)
    return false;
  textViewStatusUpdater = updater;
  return true;
}

bool textViewStatusUpdate(void) {
  if(textViewFullscreen)
    return false;
  char status[textViewStatusLen * 2 + 1] = {0};
  textViewStatusUpdater(status, textViewStatusLen * 2 + 1, textViewStatusLen);
  textViewClearStatus();
  if(!textViewPutStatus(textViewTitle, 0))
    return false;
  if(textlen(status) > textViewStatusLen)
    return false;
  if(!textViewPutStatus(status, textViewTitleLen))
    return false;
  if(textViewAllowAutoRender)
    textViewRenderStatus();
  return true;
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
    textView[(unsigned int)textViewRow * textViewCols + textViewCol] = (chr << 3) | (textViewColor & 0x3);
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
  display.setCursor(0, 0);
  textViewDraw(false);
  display.sendBuffer();
}

void textViewRenderStatus(void) {
  display.setCursor(0, 0);
  display.setDrawColor(0);
  display.drawBox(0, 0, lcdWidth, textViewChrH); // erase line in buffer 0
  textViewDraw(true);
  display.sendBuffer();
}

void textViewDraw(bool statusOnly) {
  // set the default font
  display.setFont(textViewFont);
  // Render the rows and cols matrix
  for(byte row = 0; row < (statusOnly ? 1 : textViewRows); row++)
    textViewDrawLine(row);
}

// draws a line in the currently loaded font
bool textViewDrawLine(byte row) {
  if(row > textViewRows)
    return false;
  // render the cols
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
      case TV_COLOR_F0HT:
        // Transparent background
        display.setFontMode(1);
        // Foreground white
        display.setDrawColor(0);
      break;
      case TV_COLOR_FXHT:
        // Transparent background
        display.setFontMode(1);
        // Foreground XOR
        display.setDrawColor(2);
      break;
      default:
        return false;
    }

    display.drawGlyph(col * textViewChrW, (row + 1) * textViewChrH, textView[row * textViewCols + col] >> 3);
  }
  return true;
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
