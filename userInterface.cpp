#include "userInterface.h"

void fadeLedTo(byte pin, byte fromBright, byte toBright, unsigned int duration) {
  long step = ((long)duration * 1000) / abs(fromBright - toBright);
  byte brightness = fromBright;

  while(1) {
    analogWrite(pin, brightness);

    if(toBright > fromBright) {
      if(brightness < toBright)
        brightness++;
      else
        break;
    } else if(fromBright > toBright) {
      if(brightness > toBright)
        brightness--;
      else
        break;
    }

    delayMicroseconds(step);
  }
}

void fadeLed(byte pin, bool endState, unsigned int duration, byte maxBright) {
  int brightness = endState ? 0 : maxBright;
  long step = ((long)duration * 1000) / maxBright;

  while(1) {
    analogWrite(pin, constrain(brightness, 0, maxBright));

    if(endState) {
      if(brightness < maxBright)
        brightness++;
      else
        break;
    } else {
      if(brightness > 0)
        brightness--;
      else
        break;
    }

    delayMicroseconds(step);
  }
}

byte uiMessagePrintf(const char* format, const char* buttons, ...) {
  va_list args;
  va_start(args, buttons);
  unsigned int strLength;
  if((strLength = vsnprintf(NULL, 0, format, args))) {
    va_end(args);
    char sbuf[strLength + 2];
    va_start(args, buttons);
    vsnprintf(sbuf, strLength + 1, format, args);

    unsigned int lengths[3] = {0};
    byte line = 0;
    for(unsigned int pos = 0; sbuf[pos]; pos++) {
      if(sbuf[pos] == '\n') {
        if(line < 2)
          line++;
        else
          lengths[line]++;
      } else
        lengths[line]++;
    }

    char line1[lengths[0] + 1], line2[lengths[1] + 1], line3[lengths[2] + 1];
    memset(line1, 0, lengths[0] + 1);
    memset(line2, 0, lengths[1] + 1);
    memset(line3, 0, lengths[2] + 1);

    if(lengths[0])
      memcpy(line1, sbuf, lengths[0]);
    if(lengths[1])
      memcpy(line2, sbuf + lengths[0] + 1, lengths[1]);
    if(lengths[2])
      memcpy(line3, sbuf + lengths[0] + lengths[1] + 2, lengths[2]);

    va_end(args);
    return display.userInterfaceMessage((char*)line1, (char*)line2, (char*)line3, buttons);
  }

  va_end(args);
  return 0;
}

bool doubleEntry(double* value, byte digits) {
  bool decimalPut = false;
  char key, buffer[digits + 1];
  byte cursor = 0;

  // init buffer
  memset(buffer, 0, digits + 1);
  if(!isnan(*value)) {
    if(ceilf(*value) == *value) // we have a whole number
      snprintf(buffer, digits, "%.0f", *value);
    else // we have a float
      snprintf(buffer, digits, "%f", *value);

    cursor = strlen(buffer);
    Serial.println(buffer);

    textViewPutCStr(buffer);
  } else
    *value = 0.0d;

  textViewPutCCStr("_");
  textViewSeek(-1, 0);
  textViewRender();

  while(true) {
    doEvents();
    if((key = keypad.getKey())) {
      if(key >= '0' && key <= '9') {
        if(cursor < digits) {
          buffer[cursor++] = key;
          textViewPutChr(key);

          if(cursor < digits) {
            textViewPutChr('_');
            textViewSeek(-1, 0);
          }

          textViewRender();
        }
      } else if(key == '*') {
        if(cursor) {
          bool doErase = true;
          if(buffer[cursor - 1] == '.') {
            decimalPut = false;
          } else if(buffer[cursor - 1] == '0' && cursor == 1) {
            doErase = false;
            buffer[0] = '-';
            textViewSeek(-1, 0);
            textViewPutChr('-');
          }
          if(doErase) {
            textViewSeek(-1, 0);
            buffer[--cursor] = 0;
            textViewPutCCStr("_ ");
            textViewSeek(-2, 0);
          }
          textViewRender();
        } else
          return false;
      } else if(key == '#') {
        if(decimalPut) {
          if(cursor == 1) {
            *value = 0.0d;
            textViewSeek(-1, 0);
            textViewPutCCStr("0.0");
          } else {
            *value = atof(buffer);
            if(buffer[cursor - 1] == '.')
              textViewPutChr('0');
            else {
              textViewPutChr(' ');
              textViewSeek(-1, 0);
            }
          }
          textViewLinefeed();
          textViewRender();
          return true;
        } else {
          buffer[cursor++] = '.';
          decimalPut = true;
          textViewPutCCStr("._");
          textViewSeek(-1, 0);
          textViewRender();
        }
      }
    }
  }

  return false;
}

bool intEntry(int* value, bool allowNegative) {
  byte cursor = 0;
  const byte length = 8;
  char key, buffer[length + 1] = {0};

  textViewPutChr('_');
  textViewSeek(-1, 0);
  textViewRender();

  while(true) {
    if((key = keypad.getKey())) {
      if(key >= '0' && key <= '9') {
        if(cursor < length) {
          buffer[cursor++] = key;
          textViewPutChr(key);

          if(cursor < length) {
            textViewPutChr('_');
            textViewSeek(-1, 0);
          }

          textViewRender();
        }
      } else if(key == '*') {
        if(cursor) {
          textViewSeek(-1, 0);
          if(cursor == 1 && buffer[0] == '0' && allowNegative) {
            buffer[--cursor] = '-';
            textViewPutChr('-');
          } else {
            buffer[--cursor] = 0;
            textViewPutCCStr("_ ");
            textViewSeek(-2, 0);
          }
          textViewRender();
        } else
          return false;
      } else if(key == '#') {
        if(!cursor) {
          textViewPutChr('0');
          *(value) = 0;
        }
        else
          *(value) = atoi(buffer);

        textViewPutChr(' ');
        textViewSeek(-1, 0);
        textViewLinefeed();
        textViewRender();
        return true;
      }
    }

    doEvents();
  }

  return false;
}

byte showItemMenu(char* title, byte defaultItem, const char** allItems, byte numItems, byte* shownItems, byte numShownItems) {
  return showDualItemMenu(title, defaultItem, allItems, numItems, NULL, 0, shownItems, numShownItems);
}

byte showDualItemMenu(char* title, byte defaultItem, const char** allItems1, byte numItems1, const char** allItems2, byte numItems2, byte* shownItems, byte numShownItems) {
  if(!title || !allItems1 || !numItems1 || (numItems2 && !allItems2) ||
    !shownItems || !numShownItems || numItems1 + numItems2 >= 0xFF)
    return 0;

  unsigned int menuStrLength = numShownItems, menuStrPtr = 0; // initialize with the number of linefeeds + 1 (trailing 0-byte)
  for(byte item = 0; item < numShownItems; item++) // count the required length of our string buffer
    menuStrLength += strlen(mergeItemPtr(shownItems[item], allItems1, numItems1, allItems2, numItems2));

  char menuStr[menuStrLength];
  // for every shown item do this
  for(byte item = 0; item < numShownItems; item++) {
    if(item)
      menuStr[menuStrPtr++] = '\n'; // for all items except the first, add a preceeding linefeed
    strcpy((char*)(menuStr + menuStrPtr), mergeItemPtr(shownItems[item], allItems1, numItems1, allItems2, numItems2)); // copy the string to our buffer
    menuStrPtr += strlen(mergeItemPtr(shownItems[item], allItems1, numItems1, allItems2, numItems2)); // add the length of the string, except for the zero byte that we'll overwrite
  }

  return display.userInterfaceSelectionList(title, defaultItem, menuStr);
}

bool smartMenu(const char* title, const char** items, byte itemCount, byte maxItemsListed, byte* selection) {
  return smartDualFilteredMenu(title, items, itemCount, NULL, 0, NULL, 0, maxItemsListed, selection);
}

bool smartFilteredMenu(const char* title, const char** items, byte itemCount, byte* blacklist, byte blacklistCount, byte maxItemsListed, byte* selection) {
  return smartDualFilteredMenu(title, items, itemCount, NULL, 0, blacklist, blacklistCount, maxItemsListed, selection);
}

bool smartDualFilteredMenu(const char* title, const char** items1, byte itemCount1, const char** items2, byte itemCount2, byte* blacklist, byte blacklistCount, byte maxItemsListed, byte* selection) {
  if(!title || !items1 || !itemCount1 || (itemCount2 && !items2) ||
    itemCount1 + itemCount2 >= 0xFF || !maxItemsListed || !selection)
    return false;

  const byte keyStringLength = 18;
  char keyString[keyStringLength + 1] = {0}, key;
  const byte shownIndices = 5;
  byte cursor = 0, indices[maxItemsListed], numIndices = 0;
  bool doRedraw = true;

  memset(indices, 0, maxItemsListed);
  textViewClear();
  while(true) {
    if((key = keypad.getKey())) {
      if(key >= '0' && key <= '9') {
        if(cursor < keyStringLength) {
          keyString[cursor++] = key;
          doRedraw = true;
        }
      } else if(key == '*') {
        if(cursor) {
          keyString[--cursor] = 0; // delete the previous character
          doRedraw = true; // schedule a redraw
        } else
          return false;
      } else if(key == '#') {
        if(numIndices)
          break;
      }
    }

    if(doRedraw) {
      doRedraw = false;

      textViewClear();
      textViewStatusUpdate();
      textViewPutCCStrAt(title, TV_COLOR_F0H1, 1, 0);
      textViewPutCCStrAt("> ", TV_COLOR_F1H0, 2, 0);
      textViewPutCStrAt(keyString, TV_COLOR_F1H0, 2, 2);

      // We're in the 4th (index 5) row and draw the suggestions to the 8th row (index 7)
      numIndices = findClosestIndicesDualFiltered(keyString, items1, itemCount1, items2, itemCount2, blacklist, blacklistCount, indices, maxItemsListed);
#ifdef DEBUG
      Serial.print("Key string: ");
      Serial.println(keyString);
      Serial.print("Indices: ");
      Serial.println(numIndices, DEC);
#endif
      for(byte index = 0; index < constrain(numIndices, 0, shownIndices); index++)
        textViewPutCCStrAt(mergeItemPtr(indices[index], items1, itemCount1, items2, itemCount2), TV_COLOR_F1H0, index + 3, 0);
      textViewRender();
    }

    doEvents();
  }

  textViewClear();
  textViewRender();

  if(numIndices == 1) {
    *selection = indices[0];
    keyControl();
    return true;
  } else if(numIndices > 1) {
    uiControl();
    *selection = indices[showDualItemMenu((char*)title, 1, items1, itemCount1, items2, itemCount2, indices, numIndices) - 1];
    keyControl();
    return true;
  }

  return false;
}
