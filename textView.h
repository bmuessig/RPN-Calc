#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include <Arduino.h>
#include <stdarg.h>

// Colors
enum {
  TV_COLOR_F1H0 = 0,
  TV_COLOR_F0H1 = 1,
  TV_COLOR_F1HT = 2,
  TV_COLOR_FXHT = 3
};
extern const byte textViewChrW, textViewChrH, textViewRows, textViewCols,
  textViewStatusLen, textViewTitleLen;

typedef void (*StatusUpdater)(char* statusBuffer, byte maxBufLen, byte maxTextLen);

void textViewPutStrAt(String str, byte color, byte row, byte col);
void textViewPutCStrAt(char* str, byte color, byte row, byte col);
void textViewPutCCStrAt(const char* str, byte color, byte row, byte col);
bool textViewGoto(int row, int col);
bool textViewSeek(signed char offset, char fillChar);
bool textViewSet(int row, int col, byte color);
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
bool textViewSetStatus(char* str, byte col);
bool textViewSetCTitle(char* title);
bool textViewSetCCTitle(const char* title);
bool textViewSetTitle(String title);
void textViewEraseTitle(void);
bool textViewStatusRegister(StatusUpdater updater);
bool textViewStatusUpdate(void);
bool textViewPutChr(char chr);
void textViewRender(void);
void textViewDraw(void);
unsigned int textlen(char* str);

#endif
