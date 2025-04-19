#include "../include/SirenLEDController.h"

void SirenLEDController::setup(int led1, int led2) {
    pin1 = led1;
    pin2 = led2;
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
}

void SirenLEDController::update(bool fireDetected) {
    if (fireDetected) {
        unsigned long now = millis();
        if (now - lastToggle >= 300) { // SIREN_INTERVAL
            state = !state;
            digitalWrite(pin1, state ? HIGH : LOW);
            digitalWrite(pin2, state ? LOW : HIGH);
            lastToggle = now;
        }
    } else {
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, LOW);
        state = false;
    }
}