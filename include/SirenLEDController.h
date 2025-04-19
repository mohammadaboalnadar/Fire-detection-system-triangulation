#ifndef SIREN_LED_CONTROLLER_H
#define SIREN_LED_CONTROLLER_H
#include <Arduino.h>

class SirenLEDController {
    int pin1, pin2;
    bool state = false;
    unsigned long lastToggle = 0;
public:
    void setup(int led1, int led2);
    void update(bool fireDetected);
};

#endif // SIREN_LED_CONTROLLER_H
