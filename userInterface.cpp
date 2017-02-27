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
  if(strLength = vsnprintf(NULL, 0, format, args)) {
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
  char key, buffer[digits + 1], cursor;

  // init buffer
  memset(buffer, 0, digits + 1);
  if(!isnan(*value)) {
    if(ceilf(*value) == *value) // we have a whole number
      snprintf(buffer, digits, "%.0f", *value);
    else // we have a float
      snprintf(buffer, digits, "%f", *value);

    cursor = strlen(buffer);
    textViewPutCStr(buffer);
  } else
    *value = 0.0d;

  textViewPutCStr("_");
  textViewCol--;
  textViewRender();

  while(true) {
    doEvents();
    if(key = keypad.getKey()) {
      if(key >= '0' && key <= '9') {
        if(cursor < digits) {
          buffer[cursor++] = key;
          textViewPutChr(key);

          if(cursor < digits - 1) {
            textViewPutChr('_');
            textViewCol--;
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
            buffer[cursor - 1] = '-';
            textViewCol--;
            textViewPutChr('-');
          }
          if(doErase) {
            textViewCol--;
            buffer[--cursor] = 0;
            textViewPutCStr("_ ");
            textViewCol -= 2;
          }
          textViewRender();
        } else
          return false;
      } else if(key == '#') {
        if(decimalPut) {
          if(cursor == 1) {
            *value = 0.0d;
            textViewCol--;
            textViewPutCStr("0.0");
          } else {
            *value = atof(buffer);
            if(buffer[cursor - 1] == '.')
              textViewPutChr('0');
            else {
              textViewPutChr(' ');
              textViewCol--;
            }
          }
          textViewLinefeed();
          textViewRender();
          return true;
        } else {
          buffer[cursor++] = '.';
          decimalPut = true;
          textViewPutCStr("._");
          textViewCol--;
          textViewRender();
        }
      }
    }
  }

  return false;
}

bool intEntry(int* value, bool allowNegative) {
  byte cursor = 0;
  char key, buffer[9] = {0};

  textViewPutCStr("_");
  textViewCol--;
  textViewRender();

  while(true) {
    if(key = keypad.getKey()) {
      if(key >= '0' && key <= '9') {
        if(cursor < 8) {
          buffer[cursor++] = key;
          textViewPutChr(key);

          if(cursor < 8) {
            textViewPutChr('_');
            textViewCol--;
          }

          textViewRender();
        }
      } else if(key == '*') {
        if(cursor) {
          textViewCol--;
          if(cursor == 1 && buffer[0] == '0' && allowNegative) {
            buffer[--cursor] = '-';
            textViewPutChr('-');
          } else {
            buffer[--cursor] = 0;
            textViewPutCStr("_ ");
            textViewCol -= 2;
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
        textViewCol--;
        textViewLinefeed();
        textViewRender();
        return true;
      }
    }

    doEvents();
  }

  return false;
}

byte findClosestIndices(char* keyString, const char** items, byte itemCount, byte* indices, byte maxIndices) {
  if(!keyString || !items || !itemCount || !indices || !maxIndices)
    return 0; // invalid parameters will not produce valid results

  byte indicesFound = 0, item = 0;
  while(indicesFound < maxIndices && item < itemCount) { // loop for as long as we have items or can find further items
    if(strlen(items[item]) < strlen(keyString)) { // if the string is already longer than the item, skip it for speed
#ifdef DEBUG
      Serial.println("Skipping due to length");
#endif
      item++;
      continue;
    }
    // we now know that the current items length is smaller or equal to the key sequences length

    bool isMatching = true;
    for(byte chrPos = 0; chrPos < strlen(keyString); chrPos++) { // here we loop through the letters of the key string
      char itemChr = items[item][chrPos], keyChr = keyString[chrPos];
      byte mapSegment;

      // now, match the pressed key to a map segment
      if(!matchMapSegment(keyChr, &mapSegment))
        return 0; // key string is invalid

#ifdef DEBUG
      Serial.print("Key: ");
      Serial.println(keyChr);
#endif

      // then check if the segment contains the items character
      if(!mapSegmentMatchesChar(itemChr, mapSegment)) {
        // it doesn't, so ...
        isMatching = false; // tell the succeeding code we don't have a match
        break; // and exit the string comparing loop
      }
      // it does, so ...
      // keep going
    }

    // did we have a match?
    if(isMatching)
      indices[indicesFound++] = item;

    // advance the item pointer
    item++;

    // and loop again (if possible)
  }

  return indicesFound;
}

byte showItemMenu(char* title, byte defaultItem, const char** allItems, byte numItems, byte* shownItems) {
  if(!numItems)
    return 0;

  unsigned int menuStrLength = numItems, menuStrPtr = 0; // initialize with the number of linefeeds + 1 (trailing 0-byte)
  for(byte item = 0; item < numItems; item++) // count the required length of our string buffer
    menuStrLength += strlen(allItems[shownItems[item]]);
  char menuStr[menuStrLength];
  for(byte item = 0; item < numItems; item++) {
    if(item)
      menuStr[menuStrPtr++] = '\n'; // for all items except the first, add a preceeding linefeed
    strcpy((char*)(menuStr + menuStrPtr), allItems[shownItems[item]]); // copy the string to our buffer
    menuStrPtr += strlen(allItems[shownItems[item]]); // add the length of the string, except for the zero byte that we'll overwrite
  }

  return display.userInterfaceSelectionList(title, defaultItem, menuStr);
}

bool smartMenu(const char* title, const char** items, byte itemCount, byte maxItemsListed, byte* selection) {
  if(!title || !items || !itemCount || !maxItemsListed || !selection)
    return false;

  const byte keyStringLength = 18;
  char keyString[keyStringLength + 1] = {0}, key;
  const byte shownIndices = 5;
  byte cursor = 0, indices[maxItemsListed], numIndices = 0;
  bool doRedraw = true;

  memset(indices, 0, maxItemsListed);
  textViewClear();
  while(true) {
    if(key = keypad.getKey()) {
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
      textViewPutCStrAt((char*)title, TV_COLOR_F0H1, 1, 0);
      textViewPutCStrAt("> ", TV_COLOR_F1H0, 2, 0);
      textViewPutCStrAt(keyString, TV_COLOR_F1H0, 2, 2);

      // We're in the 4th (index 5) row and draw the suggestions to the 8th row (index 7)
      numIndices = findClosestIndices(keyString, items, itemCount, indices, maxItemsListed);
#ifdef DEBUG
      Serial.print("Key string: ");
      Serial.println(keyString);
      Serial.print("Indices: ");
      Serial.println(numIndices, DEC);
#endif
      for(byte index = 0; index < constrain(numIndices, 0, shownIndices); index++)
        textViewPutCStrAt((char*)items[indices[index]], TV_COLOR_F1H0, index + 3, 0);
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
    *selection = indices[showItemMenu((char*)title, 1, items, numIndices, indices) - 1];
    keyControl();
    return true;
  }

  return false;
}

/*int fileMenu(File startDir) {
  String fileList = String("");

  while(true) {
    File entry = startDir.openNextFile();
    if(!entry)
      break;
    Serial.println(entry.name());
    if(fileList.length())
      fileList = fileList + String("\n") + String(entry.name());
    else
      fileList = String(entry.name());
  }

  char buffer[fileList.length() + 1];
  fileList.toCharArray(buffer, fileList.length() + 1);

  uiControl();
  return display.userInterfaceSelectionList("System Menu", 1, buffer);
}*/

/*void fileMenu(File dir) {
   while(true) {
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }

     if (entry.isDirectory()) {
       uiControl();
       display.userInterfaceMessage("Next entry:", entry.name(), "- Directory -", " OK ");
       fileMenu(entry);
     } else {
       char buffer[10];
       sprintf(buffer, "%d B", entry.size());
       uiControl();
       display.userInterfaceMessage("Next entry:", entry.name(), buffer, " OK ");
     }
     entry.close();
   }
}
*/
