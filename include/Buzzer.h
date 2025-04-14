#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

// Buzzer pin definition
#define BUZZER_PIN 7

// Buzzer parameters
#define BUZZER_CALIBRATION_FREQ 1500

// Function prototypes
void initializeBuzzer();
void updateBuzzer(bool flameDetected);
void playTone(unsigned int frequency, unsigned long duration);
void playStartupSequence();
void playCalibrationTone();
void playCalibrationFinishedTone();
void playCalibrationWarningTone();

#endif // BUZZER_H 