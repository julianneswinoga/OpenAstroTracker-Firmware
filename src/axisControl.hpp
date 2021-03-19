#pragma once

#include "LcdButtons.hpp"

class axisControl {
public:
    axisControl();
    bool processKeys();
    void printSubmenu();

private:
    enum axisSubmenu_t {
        AXIS_SUBMENU_PRIMARY,
        AXIS_SUBMENU_SECONDARY,
        AXIS_SUBMENU_TERTIARY,
        AXIS_SUBMENU_TARGET,
    };

    bool showTarget = true;
    axisSubmenu_t submenuSelection = AXIS_SUBMENU_PRIMARY;
};
