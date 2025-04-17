#include "../include/ServoControl.h"

ServoControl::ServoControl(int pin, int minA, int maxA, int step, int delayMs, float speed)
    : servoPin(pin), minAngle(minA), maxAngle(maxA), scanStep(step), scanDelay(delayMs), trackingSpeed(speed), scanDirection(true), lastServoUpdate(0), currentAngle(90), targetAngle(90) {}

void ServoControl::begin(int initialAngle) {
    servo.attach(servoPin);
    currentAngle = initialAngle;
    targetAngle = initialAngle;
    servo.write(currentAngle);
    lastServoUpdate = millis();
}

void ServoControl::update(bool flameDetected, float flameAngle) {
    unsigned long now = millis();
    if (!flameDetected && now - lastServoUpdate < (unsigned long)scanDelay) return;
    lastServoUpdate = now;
    if (flameDetected) {
        targetAngle = mapFlameAngleToServo(flameAngle);
        currentAngle = lerpAngle(currentAngle, targetAngle, trackingSpeed);
        servo.write(currentAngle);
    } else {
        if (scanDirection) {
            currentAngle += scanStep;
            if (currentAngle >= maxAngle) { currentAngle = maxAngle; scanDirection = false; }
        } else {
            currentAngle -= scanStep;
            if (currentAngle <= minAngle) { currentAngle = minAngle; scanDirection = true; }
        }
        servo.write(currentAngle);
    }
}

int ServoControl::getCurrentAngle() const { return currentAngle; }
int ServoControl::getTargetAngle() const { return targetAngle; }

int ServoControl::mapFlameAngleToServo(float flameAngle) {
    return map(constrain(flameAngle, -30, 30), -30, 30, maxAngle, minAngle);
}

int ServoControl::lerpAngle(int current, int target, float factor) {
    factor = constrain(factor, 0.0, 1.0);
    float result = current + factor * (target - current);
    return round(result);
}
