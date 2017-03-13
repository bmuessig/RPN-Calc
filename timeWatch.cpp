#include "timeWatch.h"
#include "timeWatchGlob.h"

int timerEndHour = -1, timerEndMinute = -1;

void timeWatch(void) {
  textViewClearAll();
  textViewSetFullscreen(false);
  keyControl();
  textViewSetCCTitle("Watch");
  textViewStatusUpdate();
  textViewLinefeed();
  textViewRender();

  bool resumeTimer = false;
  if(timerEndHour > 0 && timerEndMinute > 0) {
    textViewPutCCStr("Press any key!\n");
    textViewRender();
    awaitKey();
    uiControl();
    resumeTimer = (display.userInterfaceMessage("Resume timer?", "", "", " Resume \n New ") == 2);
    keyControl();
  } else
    resumeTimer = true;

  if(resumeTimer) {
    textViewPutCCStr("End Hour: ");
    if(!intEntry(&timerEndHour, false)) {
      uiControl();
      return;
    }
    if(timerEndHour > 23 || timerEndHour < 0) {
      uiControl();
      display.userInterfaceMessage("Error:", "Invalid hour!", "", " OK ");
      uiControl();
      return;
    }
    textViewPutCCStr("End Minute: ");
    if(!intEntry(&timerEndMinute, false)) {
      uiControl();
      return;
    }
    if(timerEndMinute > 59 || timerEndMinute < 0) {
      uiControl();
      display.userInterfaceMessage("Error:", "Invalid minute!", "", " OK ");
      uiControl();
      return;
    }
  }

  uiControl();
  bool doSleep = (display.userInterfaceMessage("Deep sleep until", "the timer runs out?", "", " Yes \n No ") == 1);
  keyControl();

  int targetTime = TIME_TO_SECS(timerEndHour, timerEndMinute, 0), curTime;
  char key;
  bool survived = true;
  analogWrite(lcdBL, 0);
  analogWrite(led, 0);
  textViewLinefeed();
  textViewSetAllowAutoRender(false);
  display.setPowerSave(true);

  while(targetTime > (curTime = CUR_TIME_TO_SECS()) && doSleep) {
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
  textViewSetAllowAutoRender(true);
  textViewStatusUpdate();
  textViewLinefeed();
  if(survived && doSleep) {
    timerEndHour = -1;
    timerEndMinute = -1;
    textViewSetColor(1);
    textViewPutCCStr("You survived!!!\n\n");
    textViewPutCCStr("\aPress any key!");
    textViewRender();
    awaitKey();
  }
  uiControl();
  delay(200);
}

bool timeWatchTimerStart(byte hour, byte minute) {
  if(hour > 23 || minute > 59)
    return false;
  timerEndHour = hour;
  timerEndMinute = minute;
  return true;
}

void timeWatchTimerStop(void) {
  timerEndHour = -1;
  timerEndMinute = -1;
}

bool timeWatchTimerRunning(void) {
  if(timerEndHour < 0 || timerEndMinute < 0)
    return false;
  bool running = (TIME_TO_SECS(timerEndHour, timerEndMinute, 0) > CUR_TIME_TO_SECS());
  if(!running) // if time is up, reset the timer
    timeWatchTimerStop();
  return running;
}

bool timeWatchTimerStatus(char* statusBuffer) {
  int secsRemaining = TIME_TO_SECS(timerEndHour, timerEndMinute, 0) - CUR_TIME_TO_SECS();
  if(!secsRemaining)
    return false;
  if(secsRemaining < 60) // less than 60 secs
    return (snprintf(statusBuffer, 6, " < 1m") > 0);
  else if(secsRemaining / 60 < 90) // less than 90 mins
    return (snprintf(statusBuffer, 6, "%4.1fm", (float)secsRemaining / 60.0f) > 0);
  else // 90 mins or more
    return (snprintf(statusBuffer, 6, "%4.1fh", (float)secsRemaining / 3600.0f) > 0);
}
