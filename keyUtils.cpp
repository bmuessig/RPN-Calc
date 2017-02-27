#include "keyUtils.h"

const byte charMapSegments = 10, charMapFields = 5;
const char charMap[charMapSegments * charMapFields] = {
  '1',   0,   0,   0,   0,      'A', 'B', 'C', '2',   0,      'D', 'E', 'F', '3',   0,
  'G', 'H', 'I', '4',   0,      'J', 'K', 'L', '5',   0,      'M', 'N', 'O', '6',   0,
  'P', 'Q', 'R', 'S', '7',      'T', 'U', 'V', '8',   0,      'W', 'X', 'Y', 'Z', '9',
                                ' ', '_', '0',   0,   0
};

void uiControl(void) {
  calmKeys(50, 500);

  pinMode(keypadRows[3], OUTPUT);
  digitalWrite(keypadRows[3], LOW);

  for(int i = 0; i < 3; i++)
    pinMode(keypadCols[i], INPUT_PULLUP);
}

void keyControl(void) {
  while(!calmKeys(50, 500));
}

char awaitKey(void) {
  char key;
  keyControl();
  while(!(key = keypad.getKey()))
    doEvents();
  keyControl();
  return key;
}

bool calmKeys(unsigned short cooldown, unsigned short timeout) {
  // Set up the matrix to be all high
  for(int i = 0; i < 3; i++)
    pinMode(keypadCols[i], INPUT_PULLUP);
  for(int i = 0; i < 4; i++) {
    pinMode(keypadRows[i], OUTPUT);
    digitalWrite(keypadRows[i], LOW);
  }

  bool success = false;
  long startTime = millis(), lastPress = millis();
  while(startTime + timeout > millis()) { // repeat for as long as nothing times out
    doEvents();
    if(!digitalRead(keypadCols[0]) || !digitalRead(keypadCols[1]) || !digitalRead(keypadCols[2]))
      lastPress = millis();
    else if(lastPress + cooldown < millis()) {
      success = true;
      break;
    }
  }

  // Restore the matrix
  for(int i = 0; i < 3; i++)
    pinMode(keypadCols[i], INPUT);
  for(int i = 0; i < 4; i++)
    pinMode(keypadRows[i], INPUT);

  return success;
}

bool matchMapSegment(char key, byte* segment) {
  if(key >= '1' && key <= '9') {
    *segment = key - '1';
    return true;
  } else if(key == '0') {
    *segment = 9;
    return true;
  } else
    return false;
}

bool mapSegmentMatchesChar(char chr, byte segment) {
  // make the letter (if applicable) uppercase
  if(chr >= 'a' && chr <= 'z')
    chr = 'A' - 'a' + chr;

#ifdef DEBUG
  Serial.print("Chr: ");
  Serial.print(chr);
#endif

  for(byte field = 0; field < charMapFields; field++) {
    if(chr == charMap[segment * charMapFields + field]) {
#ifdef DEBUG
      Serial.print(" : Match : Segment ");
      Serial.println(segment, DEC);
#endif
      return true;
    }
  }

#ifdef DEBUG
  Serial.println(": No match");
#endif
  return false;
}
