#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include <Arduino.h>
#include <stdarg.h>
#include "hardware.h"

// Colors
enum {
  TV_COLOR_F1H0,
  TV_COLOR_F0H1,
  TV_COLOR_F1HT,
  TV_COLOR_F0HT,
  TV_COLOR_FXHT
};
extern const byte textViewChrW, textViewChrH, textViewRows, textViewCols,
  textViewStatusLen, textViewTitleLen;

typedef void (*StatusUpdater)(char* statusBuffer, byte maxBufLen, byte maxTextLen);

typedef struct {
  unsigned short originX;
  unsigned short originY;
  unsigned short width;
  unsigned short height;
} GraphicsArea;

void textViewPutStrAt(String str, byte color, byte row, byte col);
void textViewPutCStrAt(char* str, byte color, byte row, byte col);
void textViewPutCCStrAt(const char* str, byte color, byte row, byte col);
bool textViewGoto(byte row, byte col);
bool textViewGraphics(byte originRow, byte originCol, byte rows, byte cols, byte color, GraphicsArea* area);
bool textViewSeek(signed char offset);
bool textViewSeekFill(signed char offset, char character, byte color);
bool textViewPaint(byte row, byte col, byte rows, byte cols, byte color);
bool textViewFill(byte row, byte col, byte rows, byte cols, char character, byte color);
bool textViewSet(byte row, byte col, byte color);
bool textViewSetColor(byte color);
void textViewSetImmLf(bool immLf);
void textViewSetFullscreen(bool fullscreen);
bool textViewIsFullscreen(void);
void textViewSetAllowAutoRender(bool allowAutoRender);
bool textViewErase(byte row, byte colStart, byte colEnd);
bool textViewEraseLine(byte row);
void textViewPutChrAt(char chr, byte color, byte row, byte col);
void textViewPutStr(String str);
void textViewPutCStr(char* str);
void textViewPutCCStr(const char* str);
unsigned int textViewPrintf(const char* format, ...);
unsigned int textViewPrintfAt(const char* format, byte color, byte row, byte col, ...);
unsigned int textViewNPrintfAt(const char* format, unsigned int maxLength, byte color, byte row, byte col, ...);
unsigned int textViewNPrintf(const char* format, unsigned int maxLength, ...);
void textViewLinefeed(void);
void textViewBackspace(void);
void textViewClear(void);
void textViewClearAll(void);
bool textViewClearStatus(void);
bool textViewPutStatus(char* str, byte col);
bool textViewSetCTitle(char* title);
bool textViewSetCCTitle(const char* title);
bool textViewSetTitle(String title);
void textViewEraseTitle(void);
bool textViewStatusRegister(StatusUpdater updater);
bool textViewStatusUpdate(void);
bool textViewPutChr(char chr);
void textViewRender(void);
void textViewRenderStatus(void);
void textViewDraw(bool statusOnly);
bool textViewDrawLine(byte row);
unsigned int textlen(char* str);

#endif
