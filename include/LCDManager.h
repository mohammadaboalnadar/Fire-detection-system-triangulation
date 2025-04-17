#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include <Arduino.h>
#include "../include/LCD.h"
#include "FlameTriangulation.h"

class LCDManager {
public:
    LCDManager(unsigned long refreshInterval);
    void update(bool flameDetected, float angle, FlameTriangulation& flameSensor);
private:
    unsigned long refreshInterval;
    unsigned long lastLCDUpdate;
    bool lastFlameState;
    float lastAngle;
};

#endif // LCD_MANAGER_H
