#include "../include/LCDManager.h"

LCDManager::LCDManager(unsigned long interval)
    : refreshInterval(interval), lastLCDUpdate(0), lastFlameState(false), lastAngle(0) {}

void LCDManager::update(bool flameDetected, float angle, FlameTriangulation& flameSensor) {
    unsigned long now = millis();
    if (flameSensor.calibrationNeeded) {
        updateLCDWithCalibrationStatus(
            flameDetected, angle, true,
            flameSensor.ambientLevel1, flameSensor.ambientLevel2, flameSensor.ambientLevel3,
            flameSensor.getCurrentAmbient1(), flameSensor.getCurrentAmbient2(), flameSensor.getCurrentAmbient3()
        );
        lastLCDUpdate = now;
        lastFlameState = flameDetected;
        lastAngle = angle;
        return;
    }
    if (now - lastLCDUpdate >= refreshInterval ||
        flameDetected != lastFlameState ||
        (flameDetected && abs(angle - lastAngle) > 3.0)) {
        updateLCD(flameDetected, angle);
        lastLCDUpdate = now;
        lastFlameState = flameDetected;
        lastAngle = angle;
    }
}
