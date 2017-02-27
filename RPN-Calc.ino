#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <SPI.h>
#include <SD.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SparkFunMAX17043.h>
#include <Keypad.h>
#include <TimeLib.h>

#include "RPN-Calc.h"
#include "textView.h"
#include "keyUtils.h"
#include "userInterface.h"
#include "schedule.h"
#include "doubleStack.h"

const char* stackOps[] = {
  "ADD",
  "SUB",
  "MUL",
  "DIV",
  "INC",
  "DEC",
  "POW",
  "EE",
  "SQ",
  "SQRT",
  "LOG10",
  "LOG",
  "ABS",
  "NEG",
  "LN",
  "PI",
  "E",
  "TAU",
  "DUP",
  "POP",
  "SWP",
  "SIN",
  "COS",
  "TAN",
  "DEG",
  "RAD",
  "ASIN",
  "ACOS",
  "ATAN",
  "FLAGS",
  "MACRO"
};

enum {
  SOP_ADD,
  SOP_SUB,
  SOP_MUL,
  SOP_DIV,
  SOP_INC,
  SOP_DEC,
  SOP_POW,
  SOP_EE,
  SOP_SQ,
  SOP_SQRT,
  SOP_LOG10,
  SOP_LOG,
  SOP_ABS,
  SOP_NEG,
  SOP_LN,
  SOP_PI,
  SOP_E,
  SOP_TAU,
  SOP_DUP,
  SOP_POP,
  SOP_SWP,
  SOP_SIN,
  SOP_COS,
  SOP_TAN,
  SOP_DEG,
  SOP_RAD,
  SOP_ASIN,
  SOP_ACOS,
  SOP_ATAN,
  SOP_FLAGS,
  SOP_MACRO,
  SOP_ELEMENT_COUNT
};

// pin 14 - Data/Command select (D/C)
// pin 8  - LCD chip select (CS)
// pin 15 - LCD reset (RST)
const byte lcdDC = 14, lcdCS = 8, lcdRS = 15, lcdBL = 9;
byte lcdContrast = 140, lcdBrightness = 255;
U8G2_PCD8544_84X48_F_4W_HW_SPI display(U8G2_R0, lcdCS, lcdDC, lcdRS);

unsigned long lastEventQuery = 0;

const char keymap[4][3] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
const byte keypadRows[4] = { 5,  4,  3,  2};
const byte keypadCols[3] = {23, 22, 21};
Keypad keypad = Keypad( makeKeymap((char*)keymap), (byte*)keypadRows, (byte*)keypadCols, 4, 3 );

const byte led = A14;
byte ledBrightness = 200;

File sdRoot;
const byte sdCS = 16;

void setup() {
  // Set up the serial port
  Serial.begin(230400);

  // Waiting for brightness setup
  uiControl();
  if(!digitalRead(keypadCols[0]))
    ledBrightness = 0;
  if(!digitalRead(keypadCols[2]))
    lcdBrightness = 0;

  // Set up the LED
  pinMode(led, OUTPUT);

  // Blink the led
  fadeLed(led, HIGH, 800, ledBrightness);

  Serial.println(F("RPN-Calculator version " VERSION " booting"));
  Serial.println();

  // Start the battery management IC
  Serial.print(F("Setting up the battery management IC..."));
  lipo.begin();
  // Improve accuracy
  lipo.quickStart();
  // Warning at 20 percent
  lipo.setThreshold(20);
  Serial.println(F(" done."));
  // Check if we can even boot
  if(lipo.getAlert()) {
    // We have a problem; the battery's dead!
    fadeLedTo(led, ledBrightness, 125, 800);
    fadeLedTo(led, 125, 0, 1400);
    Serial.print("Battery voltage (");
    Serial.print(lipo.getVoltage(), DEC);
    Serial.println("v) too low to boot.");
    Serial.println("Hanging the CPU!");
    while(1) // hang the cpu drawing as few power as possible
      delay(100);
  }

  // Initialize the SPI CS pins
  Serial.print(F("Configuring the serial peripheral interface..."));
  pinMode(sdCS, OUTPUT);
  pinMode(lcdCS, OUTPUT);
  digitalWrite(sdCS, HIGH);
  digitalWrite(lcdCS, HIGH);
  Serial.println(F(" done."));

  // Initialize the display
  Serial.print(F("Setting up the LCD screen..."));
  // Set up the backlight
  pinMode(lcdBL, OUTPUT);
  analogWrite(lcdBL, lcdBrightness);
  // Set up the display, define the last row as the UI input controls
  display.begin(keypadCols[1], keypadCols[2], keypadCols[0]);
  display.setContrast(lcdContrast);
  Serial.println(F(" done."));

  // Printing boot info
  textViewFullscreen = false;
  textViewClear();
  textViewSetStatus("Booting RPN-Calc v" VERSION, 0);
  textViewPutCStr("(c) Benedikt M. '17\n\n");
  textViewRender();

  textViewPutCStr("CPU: NXP MK20DX256\n");
  textViewPutCStr(" @96MHz, 64KB RAM\n");
  textViewPrintf("LiPo: %0.2fV (%0.0f%%)\n", lipo.getVoltage(), lipo.getSOC());
  textViewRender();
#ifndef DEBUG
  delay(1600);
#endif

  textViewPutCStr("Setting up SD: ");
  textViewRender();
  Serial.print(F("Setting up the SD card..."));
  if (!SD.begin(sdCS)) {
    textViewColor = 1;
    textViewPutCStr("failed!\n\n");
    textViewPutCStr("Boot failed. Rebooting!");
    textViewRender();
    Serial.println(F(" failed!"));
    Serial.println(F("Rebooting system!"));

    // Blink the led
    for(int i=0; i < 2; i++) {
      analogWrite(led, ledBrightness);
      delay(500);
      analogWrite(led, 0);
      delay(200);
    }

    delay(500);
    CPU_RESTART();
    return;
  }
  // Wait for the SD's ARM or 8051 CPU to become ready
#ifndef DEBUG
  delay(400);
#else
  delay(10);
#endif
  sdRoot = SD.open("/");
  textViewPutCStr("OK\n");
  Serial.println(F(" done."));

  // Initialize the RTC
  textViewPutCStr("Setting up RTC: ");
  textViewRender();
  Serial.print(F("Setting up the real time clock..."));
  setSyncProvider((getExternalTime)Teensy3Clock.get);
  textViewPutCStr("OK\n");
  Serial.println(F(" done."));

  textViewLinefeed();
  textViewColor = 1;
  textViewPutCStr("Boot complete!\a");
  textViewRender();
  Serial.println("Boot complete!");

#ifndef DEBUG
  delay(1400);
  for(int i=0; i < 5; i++) {
    analogWrite(led, ledBrightness);
    delay(30);
    analogWrite(led, 0);
    delay(30);
  }
#endif
  // Turn on the LED permanently
  analogWrite(led, ledBrightness);
#ifndef DEBUG
  delay(600);
#endif

  // Clear the display
  textViewClearAll();
  textViewRender();
}

void loop() {
  uiControl();
  mainMenu();
}

void mainMenu(void) {
  uiControl();
  switch(display.userInterfaceSelectionList("System Menu", 1, "Stack Math\nSurvive Maths\nSnake\nBasic Math\nKey Test\nReboot")) {
    case 1:
      stackMath();
      break;
    case 2:
      surviveMaths();
      break;
    case 3:
      snake();
      break;
    case 4:
      basicMath();
      break;
    case 5:
      keyTest();
      break;
    case 6:
      CPU_RESTART();
      while(1);
  }
}

///////// STACK MATH ///////

enum {
  STMATH_ERR_NONE,
  STMATH_ERR_UNDERFLOW,
  STMATH_ERR_OVERFLOW,
  STMATH_ERR_DIVZERO,
  STMATH_ERR_SQRTNEG,
  STMATH_ERR_NOTIMPL,
  STMATH_ERR_UNKNOWN
} stackMathErrors;

void stackMath(void) {
  textViewFullscreen = false;
  textViewClear();
  keyControl();
  textViewSetTitle("Stack Math");
  textViewStatusUpdate();
  textViewLinefeed();
  textViewRender();

  const byte stackSize = 16;
  char key;
  bool doRedraw = true, useRads = false;
  DoubleStack stack;
  if(!doubleStackInit(stackSize, &stack)) {
    // We don't have enough RAM left to do anything; shouldn't ever happen
    return;
  }

  while(true) {
    if(key = keypad.getKey()) {
      if(key >= '0' && key <= '9') { // enter numbers
        double value = (double)(key - '0');
        textViewClear();
        textViewStatusUpdate();
        textViewSet(7, 0, TV_COLOR_F1H0);
        textViewPutCStr("*:BKSP/-/EX, #:DP/ENT");
        textViewSet(2, 0, TV_COLOR_F1H0);
        textViewPutCStr("Push value:\n");
        if(doubleEntry(&value, 18)) {
          if(!doubleStackPush(value, &stack)) {
            uiControl();
            display.userInterfaceMessage("Error:", "Stack overflow!", "", " OK ");
          }
        }
        doRedraw = true;
        keyControl();
      } else if(key == '*') { // exit
        uiControl();
        bool doQuit = display.userInterfaceMessage("Quit?", "Really quit?", "", " No \n Yes ") == 2;
        keyControl();

        if(doQuit)
          break;
        else
          doRedraw = true;
      } else if(key == '#') { // operation menu
        byte selection;
        if(smartMenu("Select operation:", stackOps, SOP_ELEMENT_COUNT, SOP_ELEMENT_COUNT, &selection)) {
          double val1 = NAN, val2 = NAN;
          byte error = STMATH_ERR_NONE;

          // Fill the values
            switch(selection) {
              // Two values popped
              case SOP_ADD:
              case SOP_SUB:
              case SOP_MUL:
              case SOP_DIV:
              case SOP_POW:
              case SOP_LOG:
              case SOP_EE:
              case SOP_SWP:
                if(!doubleStackPop(&stack, &val1)) {
                  error = STMATH_ERR_UNDERFLOW;
                  break;
                }
                if(!doubleStackPop(&stack, &val2)) {
                  doubleStackPush(val1, &stack);
                  error = STMATH_ERR_UNDERFLOW;
                  break;
                }
              break;
              // One value popped
              case SOP_INC:
              case SOP_DEC:
              case SOP_SQ:
              case SOP_SQRT:
              case SOP_LOG10:
              case SOP_ABS:
              case SOP_NEG:
              case SOP_LN:
              case SOP_DUP:
              case SOP_POP:
              case SOP_SIN:
              case SOP_COS:
              case SOP_TAN:
              case SOP_ASIN:
              case SOP_ACOS:
              case SOP_ATAN:
              //case SOP_MACRO:
                if(!doubleStackPop(&stack, &val1)) {
                  error = STMATH_ERR_UNDERFLOW;
                  break;
                }
              break;
              case SOP_MACRO:
                error = STMATH_ERR_NOTIMPL;
              break;
            }

          if(!error) {
            switch(selection) {
                case SOP_ADD:
                  doubleStackPush(val2 + val1, &stack);
                break;
                case SOP_SUB:
                  doubleStackPush(val2 - val1, &stack);
                break;
                case SOP_MUL:
                  doubleStackPush(val2 * val1, &stack);
                break;
                case SOP_DIV:
                  if(val2 == 0) {
                    error = STMATH_ERR_DIVZERO;
                    break;
                  }
                  doubleStackPush(val2 / val1, &stack);
                break;
                case SOP_INC:
                  doubleStackPush(val1 + 1, &stack);
                break;
                case SOP_DEC:
                  doubleStackPush(val2 - 1, &stack);
                break;
                case SOP_POW:
                  doubleStackPush(pow(val2, val1), &stack);
                break;
                case SOP_EE:
                  doubleStackPush(val2 * pow(10, val1), &stack);
                break;
                case SOP_SQ:
                  doubleStackPush(sq(val1), &stack);
                break;
                case SOP_SQRT:
                  doubleStackPush(sqrt(val1), &stack);
                break;
                case SOP_LOG10:
                  doubleStackPush(log(val1) / log(10), &stack);
                break;
                case SOP_LOG:
                  doubleStackPush(log(val2) / log(val1), &stack);
                break;
                case SOP_ABS:
                  doubleStackPush(abs(val1), &stack);
                break;
                case SOP_NEG:
                  doubleStackPush(-val1, &stack);
                break;
                case SOP_LN:
                  doubleStackPush(log(val1), &stack);
                break;
                case SOP_PI:
                  doubleStackPush(PI, &stack);
                break;
                case SOP_E:
                  doubleStackPush(M_E, &stack);
                break;
                case SOP_TAU:
                  doubleStackPush(PI*2, &stack);
                break;
                case SOP_DUP:
                  doubleStackPush(val1, &stack);
                  doubleStackPush(val1, &stack);
                break;
                case SOP_POP:
                  // just put nothing back
                break;
                case SOP_SWP:
                  // put back in reverse order
                  doubleStackPush(val1, &stack);
                  doubleStackPush(val2, &stack);
                break;
                case SOP_SIN:
                  if(useRads)
                    doubleStackPush(sin(val1), &stack);
                  else
                    doubleStackPush(sin(degToRad(val1)), &stack);
                break;
                case SOP_COS:
                  if(useRads)
                    doubleStackPush(cos(val1), &stack);
                  else
                    doubleStackPush(cos(degToRad(val1)), &stack);
                break;
                case SOP_TAN:
                  if(useRads)
                    doubleStackPush(tan(val1), &stack);
                  else
                    doubleStackPush(tan(degToRad(val1)), &stack);
                break;
                case SOP_ASIN:
                  if(useRads)
                    doubleStackPush(asin(val1), &stack);
                  else
                    doubleStackPush(radToDeg(asin(val1)), &stack);
                break;
                case SOP_ACOS:
                  if(useRads)
                    doubleStackPush(acos(val1), &stack);
                  else
                    doubleStackPush(radToDeg(acos(val1)), &stack);
                break;
                case SOP_ATAN:
                  if(useRads)
                    doubleStackPush(atan(val1), &stack);
                  else
                    doubleStackPush(radToDeg(atan(val1)), &stack);
                break;
                case SOP_DEG:
                  useRads = false;
                break;
                case SOP_RAD:
                  useRads = true;
                break;
                case SOP_FLAGS:
                  uiControl();
                  uiMessagePrintf("Angle Mode: %s", " OK ", useRads ? "RAD" : "DEG");
                  keyControl();
                break;
                case SOP_MACRO:
                  error = STMATH_ERR_NOTIMPL;
                break;
                default:
                  error = STMATH_ERR_UNKNOWN;
                break;
              }
          }

          uiControl();
          switch(error) {
            case STMATH_ERR_NONE:
            break;
            case STMATH_ERR_UNDERFLOW:
              display.userInterfaceMessage("Error:", "Stack underflow!", "", " OK ");
            break;
            case STMATH_ERR_OVERFLOW:
              display.userInterfaceMessage("Error:", "Stack overflow!", "", " OK ");
            break;
            case STMATH_ERR_DIVZERO:
              display.userInterfaceMessage("Error:", "Division by 0!", "", " OK ");
            break;
            case STMATH_ERR_SQRTNEG:
              display.userInterfaceMessage("Error:", "Sqrt of neg. num!", "", " OK ");
            break;
            case STMATH_ERR_NOTIMPL:
              display.userInterfaceMessage("Error:", "Not implemented!", "", " OK ");
            break;
            case STMATH_ERR_UNKNOWN:
            default:
              display.userInterfaceMessage("Error:", "Unknown error!", "", " OK ");
            break;
          }
        }
        doRedraw = true;
        keyControl();
      }
    }

    if(doRedraw) {
      doRedraw = false;
      textViewClear();

      // Draw the stack
      double stackVal;

      if(stack.pointer) {
        textViewSet(1, 0, TV_COLOR_F1H0);
#ifdef DEBUG
        Serial.print("Stackptr: ");
        Serial.println(stack.pointer, DEC);
#endif
        for(byte ptr = 0; ptr < constrain(stack.pointer, 0, 6); ptr++) {
#ifdef DEBUG
          Serial.print("Pointer: ");
          Serial.println(ptr, DEC);
          Serial.print("Success: ");
          Serial.println(doubleStackPeekAt(ptr, &stack, &stackVal), DEC);
          Serial.print("Value: ");
          Serial.println(stackVal, DEC);
#endif
          if(doubleStackPeekAt(ptr, &stack, &stackVal))
            textViewNPrintf("%f\n", 21, stackVal);
          else
            textViewPutCStr("nan\n");
        }
      }
      textViewSet(7, 0, TV_COLOR_F1H0);
      textViewPutCStr("*:EXIT, 0-9:NUM, #:OP");

      textViewStatusUpdate();
      textViewRender();
    }

    doEvents();
  }

  doubleStackDestroy(&stack);
  textViewClearAll();
  uiControl();
}

/////// SNAKE /////

enum {
  SNAKE_CANDY_PENDING,
  SNAKE_CANDY_PRESENT
} snakeCandyStates;

enum {
  SNAKE_DIR_NONE  = 0b0000,

  SNAKE_DIR_HORIZ = 0b0001,
  SNAKE_DIR_EAST  = 0b0001,
  SNAKE_DIR_WEST  = 0b0011,

  SNAKE_DIR_VERT  = 0b0100,
  SNAKE_DIR_NORTH = 0b0100,
  SNAKE_DIR_SOUTH = 0b1100,

  SNAKE_DIR_NE    = SNAKE_DIR_NORTH | SNAKE_DIR_EAST,
  SNAKE_DIR_NW    = SNAKE_DIR_NORTH | SNAKE_DIR_WEST,

  SNAKE_DIR_SE    = SNAKE_DIR_SOUTH | SNAKE_DIR_EAST,
  SNAKE_DIR_SW    = SNAKE_DIR_SOUTH | SNAKE_DIR_WEST
} snakeDirections;

void snake(void) {
  bool stillAlive = true;

  while(stillAlive) {
    uiControl();
    switch(display.userInterfaceSelectionList("Snake", 1, "Play\nHigh scores\nQuit")) {
      case 1:
        keyControl();
        snakeGame();
      break;
      case 2:
        keyControl();
        snakeHighScores();
      break;
      case 3:
        stillAlive = false;
      break;
    }
  }

  textViewClearAll();
  textViewRender();
  uiControl();
}

void snakeGame(void) {
  textViewFullscreen = true;
  textViewClear();
  keyControl();

  const byte maxSnakeLength = 20, maxCandy = 4;
  byte snake[maxSnakeLength][2] = {0}, candy[maxCandy][4] = {0}, snakeLength = 1, direction = SNAKE_DIR_NONE;
  Schedule logicSchedule;
  unsigned int score = 0;
  bool doRedraw = true, isRunning = true;
  char key;

  // Initialize the scheduler to run every 500 ms
  scheduleInit(500, NULL, &logicSchedule);

  while(isRunning) {
    // Check for key presses every cycle
    switch(key = keypad.getKey()) {
      case '1':
        direction = SNAKE_DIR_NW;
      break;
      case '2':
        direction = SNAKE_DIR_NORTH;
      break;
      case '3':
        direction = SNAKE_DIR_NE;
      break;
      case '4':
        direction = SNAKE_DIR_WEST;
      break;
      case '5':
        direction = SNAKE_DIR_NONE; // this is essentially cheating; will be removed later
      break;
      case '6':
        direction = SNAKE_DIR_EAST;
      break;
      case '7':
        direction = SNAKE_DIR_SW;
      break;
      case '8':
        direction = SNAKE_DIR_SOUTH;
      break;
      case '9':
        direction = SNAKE_DIR_SE;
      break;
      case '0':
        // not yet defined; maybe some special action?
      break;
      case '*':
        // will exit the game
        isRunning = false;
      break;
      case '#':
        // will pause the game
      break;
    }

    // Run the game logic every n-milliseconds
    if(scheduleRun(&logicSchedule)) {
      Serial.println("Event!");
    }

    if(doRedraw) {
      doRedraw = true;
      textViewClear();

      // Render the playfield
      // First, render the snake itself
      for(byte segment = 0; segment < snakeLength; segment++)
        textViewPutChrAt(segment ? 'Q' : '*', segment ? TV_COLOR_F1H0 : TV_COLOR_F0H1, snake[segment][0], snake[segment][1]);

      // Now render the candy
      for(byte berry = 0; berry < maxCandy; berry++) {
        if(candy[berry][0] == SNAKE_CANDY_PRESENT)
          textViewPutChrAt('$', TV_COLOR_F0H1, candy[berry][2], candy[berry][3]);
      }

      textViewRender();
    }
  }

}

void snakeHighScores(void) {

}

////// SURVIVE MATHS /////

int surviveEndHour = -1, surviveEndMinute = -1;
#define TIME_TO_SECS(h, m, s) (h * 3600 + m * 60 + s)
#define CUR_TIME_TO_SECS() TIME_TO_SECS(hour(now()), minute(now()), second(now()))

void surviveMaths(void) {
  textViewFullscreen = false;
  textViewClear();
  keyControl();
  textViewSetTitle("Survive IT!");
  textViewStatusUpdate();
  textViewLinefeed();
  textViewRender();
  bool resumeTimer = false;
  if(surviveEndHour > 0 && surviveEndMinute > 0) {
    textViewPutCStr("Press any key!\n");
    textViewRender();
    awaitKey();
    uiControl();
    resumeTimer = (display.userInterfaceMessage("Resume timer?", "", "", " Resume \n New ") == 2);
    keyControl();
  } else
    resumeTimer = true;

  if(resumeTimer) {
    textViewPutCStr("End Hour: ");
    if(!intEntry(&surviveEndHour, false)) {
      uiControl();
      return;
    }
    if(surviveEndHour > 23 || surviveEndHour < 0) {
      uiControl();
      display.userInterfaceMessage("Error:", "Invalid hour!", "", " OK ");
      uiControl();
      return;
    }
    textViewPutCStr("End Minute: ");
    if(!intEntry(&surviveEndMinute, false)) {
      uiControl();
      return;
    }
    if(surviveEndMinute > 59 || surviveEndMinute < 0) {
      uiControl();
      display.userInterfaceMessage("Error:", "Invalid minute!", "", " OK ");
      uiControl();
      return;
    }
  }

  int targetTime = TIME_TO_SECS(surviveEndHour, surviveEndMinute, 0), curTime;
  char key;
  bool survived = true;
  analogWrite(lcdBL, 0);
  analogWrite(led, 0);
  textViewLinefeed();
  textViewAllowAutoRender = false;
  display.setPowerSave(true);

  while(targetTime > (curTime = CUR_TIME_TO_SECS())) {
    doEvents();
    if((key = keypad.getKey()) && key == '#') {
      display.setPowerSave(false);
      char remainingStr[20] = {0};
      sprintf(remainingStr, "%.4d min remaining", (targetTime - curTime) / 60);
      textViewSet(-1, 0, 1);
      textViewPutCStr(remainingStr);
      textViewStatusUpdate();
      textViewRender();
      delay(2000);
      display.setPowerSave(true);
    } else if(key == '*') {
      survived = false;
      break;
    }
  }

  analogWrite(lcdBL, lcdBrightness);
  analogWrite(led, ledBrightness);
  display.setPowerSave(false);
  textViewAllowAutoRender = true;
  textViewStatusUpdate();
  textViewLinefeed();
  if(survived) {
    surviveEndHour = -1;
    surviveEndMinute = -1;
    textViewColor = 1;
    textViewPutCStr("You survived!!!\n\n");
    textViewPutCStr("\aPress any key!");
    textViewRender();
    awaitKey();
  }
  uiControl();
  delay(200);
}

void basicMath(void) {
  char key;
  double num1 = NAN, num2 = NAN, res = 0;
  bool success = true;

  textViewClearAll();
  keyControl();
  textViewSetTitle("Basic Math");
  textViewStatusUpdate();
  textViewLinefeed();
  textViewPutCStr("First number:\n");
  textViewRender();

  keyControl();
  if(!doubleEntry(&num1, 20))
    return;
  keyControl();
  textViewPutCStr("\nSecond number:\n");
  if(!doubleEntry(&num2, 20))
    return;
  keyControl();
  delay(100);
  uiControl();
  switch(display.userInterfaceSelectionList("Choose operation", 1, "Add\nSubtract\nMultiply\nDivide\nPower of")) {
    case 1:
      res = num1 + num2;
      break;
    case 2:
      res = num1 - num2;
      break;
    case 3:
      res = num1 * num2;
      break;
    case 4:
      if(num2 == 0)
        success = false;
      else
        res = num1 / num2;
      break;
    case 5:
      res = pow(num1, num2);
      break;
  }
  keyControl();

  if(success) {
    textViewPutCStr("\nResult:\n");
    textViewPutStr(String(res, DEC));
  } else
    textViewPutCStr("\n\aAn error occured.\a");
  textViewPutCStr("\nPress any key.");
  textViewRender();
  awaitKey();
  textViewClearAll();
  textViewRender();
  keyControl();
  delay(100);
}

void keyTest(void) {
  char key;
  bool lastStar = false;

  textViewClearAll();
  keyControl();
  textViewSetTitle("Key test");
  textViewStatusUpdate();
  textViewLinefeed();
  textViewRender();

  while(true) {
    doEvents();

    if(key = keypad.getKey()) {
      textViewPutChr(key);
      textViewRender();

      if(key == '*') {
        if(lastStar) {
          keyControl();
          break;
        } else
          lastStar = true;
      } else
        lastStar = false;
    }
  }

  textViewClearAll();
  textViewRender();
  uiControl();
}

double radToDeg(double degVal) {
  return degVal * (180 / PI);
}

double degToRad(double radVal) {
  return radVal * (PI / 180);
}

// Do events in a blocking loop
void doEvents(void) {
  // run every 10 seconds
  if(millis() - lastEventQuery > 10000 || lastEventQuery == 0) {
    Serial.println("Updating display status bar...");
    textViewStatusUpdate();
    if(textViewAllowAutoRender)
      textViewRender();
    lastEventQuery = millis();
  }
}

void printDirectory(File dir, int numTabs) {
   while(true) {
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}
