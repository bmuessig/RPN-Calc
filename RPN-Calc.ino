#include "RPN-Calc.h"
#include "textView.h"
#include "keyUtils.h"
#include "userInterface.h"
#include "schedule.h"

#include "stackMathGlob.h"
#include "snakeGlob.h"

// pin 14 - Data/Command select (D/C)
// pin 8  - LCD chip select (CS)
// pin 15 - LCD reset (RST)
const byte lcdDC = 14, lcdCS = 8, lcdRS = 15, lcdBL = 9;
byte lcdContrast = 140, lcdBrightness = 255;
U8G2_PCD8544_84X48_F_4W_HW_SPI display(U8G2_R0, lcdCS, lcdDC, lcdRS);

Schedule eventSchedule;

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

  // Set up the event scheduler
  scheduleInit(10000, NULL, &eventSchedule);
}

void loop() {
  uiControl();
  mainMenu();
}

void mainMenu(void) {
  uiControl();
  switch(display.userInterfaceSelectionList("System Menu", 1, "Stack Math\nSurvive Maths\nPlay Snake\nReboot")) {
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
      CPU_RESTART();
      while(1);
  }
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

// Do events in a blocking loop
void doEvents(void) {
  if(scheduleRun(&eventSchedule)) {
    Serial.println("Updating display status bar...");
    textViewStatusUpdate();
    if(textViewAllowAutoRender)
      textViewRender();
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
