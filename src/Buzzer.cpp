#include "../include/Buzzer.h"

// Initialize buzzer pin
void initializeBuzzer() {
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);
}

// Function to update buzzer based on detection status
void updateBuzzer(bool flameDetected) {
  static unsigned long lastSirenUpdate = 0;
  static bool sirenState = false;
  
  if (flameDetected) {
    // Create a siren effect - alternating between two frequencies
    const unsigned int lowFreq = 800;  // Low siren tone
    const unsigned int highFreq = 2000; // High siren tone
    const unsigned long sirenInterval = 300; // Time between tone changes (milliseconds)
    
    // Non-blocking siren pattern
    if (millis() - lastSirenUpdate >= sirenInterval) {
      lastSirenUpdate = millis();
      sirenState = !sirenState;
      
      if (sirenState) {
        // Rising tone
        tone(BUZZER_PIN, highFreq);
      } else {
        // Falling tone
        tone(BUZZER_PIN, lowFreq);
      }
    }
    
    // Adjust LED blink rate to match siren (optional)
    // digitalWrite(LED_STATUS, sirenState);
  } else {
    // No flame detected - silence the buzzer
    noTone(BUZZER_PIN);
  }
}

// Function to play a tone for a specific duration
void playTone(unsigned int frequency, unsigned long duration) {
  tone(BUZZER_PIN, frequency, duration);
  delay(duration);
  noTone(BUZZER_PIN);
}

// Play a startup sequence
void playStartupSequence() {
  // Note durations
  const int sixteenth = 100;
  const int eighth = 200;
  const int quarter = 400;
  const int halfnote = 800;
  const int gap = 20;
  
  playTone(1175, sixteenth); // D
  delay(gap/2);
  playTone(1175, sixteenth); // D
  delay(gap);
  
  playTone(2349, sixteenth); // D^
  delay(gap + sixteenth);
  
  playTone(1760, sixteenth); // A
  delay(gap + eighth + sixteenth);
  
  playTone(1661, sixteenth); // G#
  delay(gap + sixteenth);
  
  playTone(1568, sixteenth); // G
  delay(gap + sixteenth);
  
  playTone(1397, sixteenth); // F
  delay(gap + sixteenth);
  
  playTone(1175, sixteenth); // D
  delay(gap);
  
  playTone(1397, sixteenth); // F
  delay(gap);
  
  playTone(1568, sixteenth); // G
  delay(gap);
  
  noTone(BUZZER_PIN);
}

// Play calibration tone
void playCalibrationTone() {
  // Alternating tones to indicate calibration
  for (int i = 0; i < 2; i++) {
    tone(BUZZER_PIN, BUZZER_CALIBRATION_FREQ, 100);
    delay(150);
    tone(BUZZER_PIN, BUZZER_CALIBRATION_FREQ - 300, 100);
    delay(150);
  }
  noTone(BUZZER_PIN);
}

// Play calibration finished tone
void playCalibrationFinishedTone() {
  for (int i = 1; i < 4; i++) {
    tone(BUZZER_PIN, i * 1000, 100);
    delay(125);
  }
  noTone(BUZZER_PIN);
}

// Play calibration warning tone (double beep)
void playCalibrationWarningTone() {
  // Short double beep to alert user about needed calibration
  tone(BUZZER_PIN, 2000, 50);
  delay(70);
  tone(BUZZER_PIN, 2000, 50);
  delay(70);
  noTone(BUZZER_PIN);
} 