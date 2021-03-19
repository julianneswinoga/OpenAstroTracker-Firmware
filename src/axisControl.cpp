#include "axisControl.hpp"

#include "b_setup.hpp"

//void processSubmenuChange(axisSubmenu_t selection) {
//    switch (selection) {
//        case axisCon
//    }
//}

bool axisControl::processKeys() {
    const lcdButton_t currentKeyPressed = lcdButtons.currentState();
    if (currentKeyPressed == btnUP || currentKeyPressed == btnDOWN) {
//        processSubmenuChange(submenuSelection);
    }
    return true;
}

void axisControl::printSubmenu() {

}

axisControl::axisControl() {

}
