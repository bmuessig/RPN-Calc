#include "timeWatch.h"
#include "timeWatchGlob.h"

int timerEndHour = -1, timerEndMinute = -1;

void timeWatch(void) {
  textViewClearAll();
  keyControl();
  textViewSetCCTitle("Watch");
  textViewStatusUpdate();
  textViewLinefeed();

  GraphicsArea clockArea;
  bool resrg = textViewGraphics(1, 1, 5, 10, TV_COLOR_F1HT, &clockArea);
#ifdef DEBUG
  Serial.print("Gfx success: ");
  Serial.println(resrg, DEC);
#endif
  bool resrc = timeWatchRenderClock(&clockArea, false, 12, 30, 25);
#ifdef DEBUG
  Serial.print("Clock success: ");
  Serial.println(resrc, DEC);
#endif
  textViewRender();
  awaitKey();

  bool resumeTimer = false;
  if(timerEndHour > 0 && timerEndMinute > 0) {
    textViewPutCCStr("Press any key!\n");
    textViewRender();
    awaitKey();
    uiControl();
    byte action = display.userInterfaceMessage("Timer already running.", "What should be done?", "", "Resume\nStop\nNew");
    if(action == 1)
      resumeTimer = true;
    else if(action == 2) {
      timeWatchTimerStop();
      return;
    }
    keyControl();
  }

  if(!resumeTimer) {
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
  textViewLinefeed();
  textViewSetAllowAutoRender(false);
  hwDeepSleep(true);

  while(targetTime > (curTime = CUR_TIME_TO_SECS()) && doSleep) {
    handleEvents();
    if((key = keypad.getKey()) && key == '#') {
      hwDeepSleep(false);
      char timeStr[7] = {0};
      timeWatchTimerStatus(timeStr);
      textViewSet(-1, 0, 1);
      textViewPutCStr(timeStr);
      textViewPutCCStr(" remaining");
      textViewStatusUpdate();
      textViewRender();
      delay(2000);
      hwDeepSleep(true);
    } else if(key == '*') {
      survived = false;
      break;
    }
  }

  hwDeepSleep(false);
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

bool timeWatchRenderClock(GraphicsArea* renderArea, bool invertColors,
  byte hour, byte minute, byte second) {
  bool showSecond = (second != 0xFF), showMinute = (minute != 0xFF);
  if(!renderArea || hour > 23 || (minute > 59 && showMinute) || (second > 59 && showSecond)) {
#ifdef DEBUG
  Serial.println("Invalid function args!");
#endif
    return false;
  }
  if(!(renderArea->width) || !(renderArea->height)) {
#ifdef DEBUG
    Serial.println("Invalid render area!");
#endif
    return false;
  }
  byte boxSize, boxOriginX = 0, boxOriginY = 0;
  if(renderArea->width < renderArea->height) {
    boxSize = renderArea->width;
    boxOriginY = (renderArea->height - renderArea->width) / 2;
  } else {
    boxSize = renderArea->height;
    boxOriginX = (renderArea->width - renderArea->height) / 2;
  }
  boxOriginX += renderArea->originX;
  boxOriginY += renderArea->originY;
  byte boxCenterX = boxOriginX + boxSize / 2, boxCenterY = boxOriginY + boxSize / 2;
  display.setDrawColor(!invertColors);
  display.drawCircle(boxCenterX, boxCenterY, boxSize / 2, U8G2_DRAW_ALL);
  return true;
}

bool timeWatchTimerStart(byte hour, byte minute) {
  if(hour > 23 || minute > 59)
    return false;
  timerEndHour = hour;
  timerEndMinute = minute;
  return timeWatchTimerRunning();
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
