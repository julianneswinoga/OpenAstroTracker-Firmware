#pragma once

#if DISPLAY_TYPE > 0
hmsSelect_t DECselect = HMS_SELECT_HOURS;
bool showTargetDEC = true;

bool processDECKeys() {
  lcdButton_t key;
  bool waitForRelease = false;
  if (lcdButtons.currentState() == btnUP) {
    if (DECselect == 0) { mount.targetDEC().addDegrees(1); showTargetDEC = true; }
    if (DECselect == 1) { mount.targetDEC().addMinutes(1); showTargetDEC = true; }
    if (DECselect == 2) { mount.targetDEC().addSeconds(1); showTargetDEC = true; }
    if (DECselect == 3) { showTargetDEC = !showTargetDEC; waitForRelease=true; }
    // slow down key repetitions
    mount.delay(200);
  }
  else if (lcdButtons.currentState() == btnDOWN) {
    if (DECselect == 0) { mount.targetDEC().addDegrees(-1); showTargetDEC = true; }
    if (DECselect == 1) { mount.targetDEC().addMinutes(-1); showTargetDEC = true; }
    if (DECselect == 2) { mount.targetDEC().addSeconds(-1); showTargetDEC = true; }
    if (DECselect == 3) { showTargetDEC = !showTargetDEC; waitForRelease=true; }
    // slow down key repetitions
    mount.delay(200);
  }
  else if (lcdButtons.keyChanged(&key)) {
    waitForRelease = true;
    switch (key)
    {
      case btnLEFT:
        DECselect = static_cast<hmsSelect_t>(adjustWrap(static_cast<int>(DECselect),
                                                        1,
                                                        static_cast<int>(HMS_SELECT_HOURS),
                                                        static_cast<int>(HMS_SELECT_TARGET)));
      break;

      case btnSELECT:
        if (mount.isSlewingRAorDEC()) {
          mount.stopSlewing(ALL_DIRECTIONS);
        }
        else {
          mount.startSlewingToTarget();
        }
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

void printDECSubmenu() {
  if (mount.isSlewingIdle()) {
    String dec = mount.DECString(LCDMENU_STRING | (showTargetDEC ? TARGET_STRING : CURRENT_STRING), DECselect).substring(0,13);
    dec += (DECselect == 3) ? ">" : " ";
    dec += showTargetDEC? "Ta" : "Cu";
    lcdMenu.printMenu(dec);
  }
}

#endif
