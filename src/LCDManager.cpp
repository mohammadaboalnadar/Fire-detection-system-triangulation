#include "../include/LCDManager.h"

LCDManager::LCDManager(unsigned long interval)
    : refreshInterval(interval), lastLCDUpdate(0), lastFlameState(false), lastAngle(0), dhtInitialized(false) {}

void LCDManager::update(bool flameDetected, float angle, FlameTriangulation& flameSensor) {
    // Initialize DHT sensor on first call
    if (!dhtInitialized) {
        initializeDHT();
        dhtInitialized = true;
    }

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
        
        // Use temperature/humidity display when no flame detected
        if (!flameDetected) {
            updateLCDWithTempHumidity(flameDetected, angle);
        } else {
            updateLCD(flameDetected, angle);
        }
        
        lastLCDUpdate = now;
        lastFlameState = flameDetected;
        lastAngle = angle;
    }
}
