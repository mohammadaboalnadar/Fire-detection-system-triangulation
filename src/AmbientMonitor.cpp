#include "../include/AmbientMonitor.h"
#include "../include/Buzzer.h"

AmbientMonitor::AmbientMonitor(unsigned long interval)
    : checkInterval(interval), lastAmbientCheck(0) {}

void AmbientMonitor::update(FlameTriangulation& flameSensor) {
    unsigned long now = millis();
    if (now - lastAmbientCheck >= checkInterval) {
        lastAmbientCheck = now;
        flameSensor.updateCalibrationMonitoring();
        if (flameSensor.calibrationNeeded && !flameSensor.calibrationWarningTriggered) {
            flameSensor.calibrationWarningTriggered = true;
            playCalibrationWarningTone();
            Serial.println(F("CALIBRATION WARNING: Ambient drift detected!"));
        }
    }
}
