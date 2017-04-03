#include "settingsView.h"
#include "settingsViewGlob.h"

void settingsView(void) {
  textViewSetFullscreen(false);
  textViewClear();
  uiControl();

  bool isRunning = true;
  while(isRunning) {
    uiControl();
    switch(display.userInterfaceSelectionList("Settings", 0, "Reset time\nQuit")) {
      case 1:
        rtcReset();
        display.userInterfaceMessage("Notice", "Time reset!", "", " OK ");
      break;
      case 2:
        isRunning = false;
      break;
    }
  }

  textViewClearAll();
  textViewRender();
  uiControl();
}
