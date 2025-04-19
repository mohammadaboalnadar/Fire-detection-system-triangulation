#include "../include/PumpControl.h"

PumpControl::PumpControl(int relay, float threshold, unsigned long pulseDur, unsigned long pulseDel)
    : relayPin(relay), angleThreshold(threshold), pulseDuration(pulseDur), pulseDelay(pulseDel), pumpEnabled(false), pumpActive(false), pumpStateChangeTime(0) {}

void PumpControl::begin() {
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, HIGH); // Ensure pump is off (active low)
    pumpEnabled = false;
    pumpActive = false;
    pumpStateChangeTime = millis();
}

void PumpControl::update(bool flameDetected, int servoAngle, int targetServoAngle) {
    bool shouldEnable = false;
    if (flameDetected) {
        int angleDifference = abs(servoAngle - targetServoAngle);
        shouldEnable = (angleDifference <= angleThreshold);
    }
    pumpEnabled = shouldEnable;
    if (pumpEnabled) {
        unsigned long now = millis();
        if (now - pumpStateChangeTime >= (pumpActive ? pulseDuration : pulseDelay)) {
            pumpActive = !pumpActive;
            digitalWrite(relayPin, pumpActive ? LOW : HIGH);
            pumpStateChangeTime = now;
        }
    } else {
        if (pumpActive) {
            pumpActive = false;
            digitalWrite(relayPin, HIGH);
        }
    }
}

bool PumpControl::isPumpActive() const { return pumpActive; }
bool PumpControl::isPumpEnabled() const { return pumpEnabled; }
