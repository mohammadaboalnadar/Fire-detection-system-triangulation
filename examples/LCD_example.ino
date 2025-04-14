/**
 * LCD Example - Demonstrates how to use the LCD module
 * 
 * This example shows how to:
 * 1. Initialize the LCD
 * 2. Display startup message
 * 3. Show fire detection information
 * 4. Use non-blocking text scrolling
 */

#include "../include/LCD.h"

// Long message for scrolling demo
const String longMessage = "This is a long message that will scroll across the LCD display. It demonstrates the non-blocking scrolling function.";

// Variables for demo
bool flameDetected = false;
float angle = 0.0;
unsigned long lastToggleTime = 0;
unsigned long lastUpdateTime = 0;

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  Serial.println(F("LCD Example Starting..."));
  
  // Initialize the LCD
  initializeLCD();
  
  // Show calibration message for 2 seconds
  displayCalibrationMessage();
  delay(2000);
  
  // Clear LCD to prepare for main loop
  clearLCD();
}

void loop() {
  // Toggle flame detection every 5 seconds for demo
  if (millis() - lastToggleTime >= 5000) {
    lastToggleTime = millis();
    flameDetected = !flameDetected;
    
    // Generate random angle when "flame" is detected
    if (flameDetected) {
      angle = random(-30, 30);
      Serial.println(F("Flame detected!"));
    } else {
      Serial.println(F("No flame detected"));
    }
    
    // Update the LCD with the new flame detection status
    updateLCD(flameDetected, angle);
  }
  
  // Every 10 seconds, demonstrate scrolling text
  if (millis() - lastUpdateTime >= 10000) {
    lastUpdateTime = millis();
    
    clearLCD();
    lcd.setCursor(0, 0);
    lcd.print("Scrolling Demo:");
    
    // Start scrolling text on the second line
    Serial.println(F("Starting scrolling text demo"));
  }
  
  // Call this in every loop iteration to keep the scrolling working
  // Note how this is non-blocking - it doesn't use delay()
  scrollLongText(longMessage, 1, 300);
} 