#include "RPN-Calc.h"

Schedule eventSchedule;

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
    Serial.print(F("Battery voltage ("));
    Serial.print(lipo.getVoltage(), DEC);
    Serial.println(F("v) too low to boot."));
    Serial.println(F("Hanging the CPU!"));
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
  textViewSetFullscreen(false);
  textViewClear();
  textViewSetStatus((char*)("Booting RPN-Calc v" VERSION), 0);
  textViewPutCCStr("(c) Benedikt M. '17\n\n");
  textViewRender();

  textViewPutCCStr("CPU: " PROCESSOR "\n");
  textViewPutCCStr(" @" STR(CLOCKSPEED) "MHz, " STR(MEMORY) "KB RAM\n");
  textViewPrintf("LiPo: %0.2fV (%0.0f%%)\n", lipo.getVoltage(), lipo.getSOC());
  textViewRender();
#ifndef DEBUG
  delay(1600);
#endif

  textViewPutCCStr("Setting up SD: ");
  textViewRender();
  Serial.print(F("Setting up the SD card..."));
  if (!SD.begin(sdCS)) {
    textViewSetColor(1);
    textViewPutCCStr("failed!\n\n");
    textViewPutCCStr("Boot failed. Rebooting!");
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
  textViewPutCCStr("OK\n");
  Serial.println(F(" done."));

  // Initialize the RTC
  textViewPutCCStr("Setting up RTC: ");
  textViewRender();
  Serial.print(F("Setting up the real time clock..."));
  setSyncProvider((getExternalTime)Teensy3Clock.get);
  textViewPutCCStr("OK\n");
  Serial.println(F(" done."));

  textViewLinefeed();
  textViewSetColor(1);
  textViewPutCCStr("Boot complete!\a");
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

  // Set up the status handler
  textViewStatusRegister(&doStatus);
}

void loop() {
  uiControl();
  mainMenu();
}

void mainMenu(void) {
  uiControl();
  switch(display.userInterfaceSelectionList("System Menu", 1, "Calculation\nWatch\nSnake game\nSettings\nReboot")) {
    case 1:
      stackMath();
    break;
    case 2:
      timeWatch();
    break;
    case 3:
      snake();
    break;
    case 4:
      settingsView();
    break;
    case 5:
      CPU_RESTART();
      while(1);
  }
}

void doStatus(char* statusBuffer, byte statusBufferLength, byte statusTextLength) {
  char batt = map(lipo.getSOC(), 0, 100, '0', '9');
  snprintf(statusBuffer, statusBufferLength, "\a \a[%c] %02d.%02d", batt, hour(now()), minute(now()));
}

// Do events in a blocking loop
void doEvents(void) {
  if(scheduleRun(&eventSchedule)) {
    Serial.println(F("Updating display status bar..."));
    textViewStatusUpdate();
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
