#pragma once

#if DISPLAY_TYPE > 0
hmsSelect_t RAselect = HMS_SELECT_HOURS;
bool showTargetRA = true;

bool processRAKeys() {
  lcdButton_t key;
  const lcdButton_t currentKeyPressed = lcdButtons.currentState();
  bool waitForRelease = false;
  if (currentKeyPressed == btnUP) {
    if (RAselect == HMS_SELECT_HOURS) { mount.targetRA().addHours(1); showTargetRA = true; }
    if (RAselect == HMS_SELECT_MINS) { mount.targetRA().addMinutes(1); showTargetRA = true; }
    if (RAselect == HMS_SELECT_SECS) { mount.targetRA().addSeconds(1); showTargetRA = true; }
    if (RAselect == HMS_SELECT_TARGET) { showTargetRA = !showTargetRA; waitForRelease=true; }

    // slow down key repetitions
    mount.delay(200);
  }
  else if (currentKeyPressed == btnDOWN) {
    if (RAselect == HMS_SELECT_HOURS) { mount.targetRA().addHours(-1); showTargetRA = true; }
    if (RAselect == HMS_SELECT_MINS) { mount.targetRA().addMinutes(-1); showTargetRA = true; }
    if (RAselect == HMS_SELECT_SECS) { mount.targetRA().addSeconds(-1); showTargetRA = true; }
    if (RAselect == HMS_SELECT_TARGET) { showTargetRA = !showTargetRA; waitForRelease=true; }

    // slow down key repetitions
    mount.delay(200);
  }
  else if (lcdButtons.keyChanged(&key)) {
    waitForRelease = true;
    switch (key)
    {
      case btnLEFT:
        RAselect = static_cast<hmsSelect_t>(adjustWrap(static_cast<int>(RAselect),
                                                       1,
                                                       static_cast<int>(HMS_SELECT_HOURS),
                                                       static_cast<int>(HMS_SELECT_TARGET)));
      break;

      case btnSELECT:
        if (mount.isSlewingRAorDEC()) {
          mount.stopSlewing(ALL_DIRECTIONS);
          mount.waitUntilStopped(ALL_DIRECTIONS);
        }

        mount.startSlewingToTarget();
      break;

      case btnRIGHT:
        lcdMenu.setNextActive();
      break;

      default:
      break;
    }
  }

  return waitForRelease;
}

void printRASubmenu() {
  if (mount.isSlewingIdle()) {
    String ra = mount.RAString(LCDMENU_STRING | (showTargetRA ? TARGET_STRING : CURRENT_STRING), RAselect).substring(0,12);
    ra += (RAselect == 3) ? ">" : " ";
    ra += showTargetRA ? "Ta" : "Cu";
    lcdMenu.printMenu(ra);
  }
}
#endif
