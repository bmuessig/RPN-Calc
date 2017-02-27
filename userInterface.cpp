

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

bool doubleEntry(double* value, byte digits)
{
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

bool intEntry(int* value, bool allowNegative)
{
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
