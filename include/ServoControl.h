#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>
#include <Servo.h>

class ServoControl {
public:
    ServoControl(int pin, int minAngle, int maxAngle, int scanStep, int scanDelay, float trackingSpeed);
    void begin(int initialAngle);
    void update(bool flameDetected, float flameAngle);
    int getCurrentAngle() const;
    int getTargetAngle() const;
private:
    Servo servo;
    int servoPin;
    int minAngle, maxAngle, scanStep, currentAngle, targetAngle;
    float trackingSpeed;
    bool scanDirection;
    unsigned long lastServoUpdate;
    int scanDelay;
    int mapFlameAngleToServo(float flameAngle);
    int lerpAngle(int current, int target, float factor);
};

#endif // SERVO_CONTROL_H
