#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H

#include <Arduino.h>

class PumpControl {
public:
    PumpControl(int relayPin, float angleThreshold, unsigned long pulseDuration, unsigned long pulseDelay);
    void begin();
    void update(bool flameDetected, int servoAngle, int targetServoAngle);
    bool isPumpActive() const;
    bool isPumpEnabled() const;
private:
    int relayPin;
    float angleThreshold;
    unsigned long pulseDuration, pulseDelay;
    bool pumpEnabled, pumpActive;
    unsigned long pumpStateChangeTime;
};

#endif // PUMP_CONTROL_H
