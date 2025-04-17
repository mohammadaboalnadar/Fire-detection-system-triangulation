#include <Arduino.h>
#include <Servo.h>
#include "../include/FlameTriangulation.h"
#include "../include/Buzzer.h"
#include "../include/LCD.h"
#include "../include/ServoControl.h"
#include "../include/PumpControl.h"
#include "../include/AmbientMonitor.h"
#include "../include/LCDManager.h"

// Pin definitions
#define SENSOR1_PIN A2  // Right sensor
#define SENSOR2_PIN A0  // Left sensor
#define SENSOR3_PIN A1  // Middle sensor

// Indicator LEDs
#define LED_STATUS 13
#define CALIBRATION_BUTTON 2
#define SERVO_PIN 9

// Pump control relay
#define PUMP_RELAY_PIN 6

// Servo scanning parameters
#define SCAN_MIN_ANGLE 30
#define SCAN_MAX_ANGLE 150
#define SCAN_STEP 1       // Degrees per step
#define SCAN_DELAY 30     // Milliseconds between steps
#define TRACKING_SPEED 0.1 // Lerp factor (0.0-1.0) - higher values = faster tracking

// LCD refresh parameters
#define LCD_REFRESH_INTERVAL 500  // Minimum time between LCD updates in milliseconds

// Ambient monitoring parameters
#define AMBIENT_CHECK_INTERVAL 5000  // Check for ambient drift every 5 seconds

// Pump control parameters
#define PUMP_ANGLE_THRESHOLD 7.0     // Activate pump when within +/- degrees of target
#define PUMP_PULSE_DURATION 1000      // Duration of water pulse in milliseconds
#define PUMP_PULSE_DELAY 1000        // Delay between pulses in milliseconds

// Global objects
FlameTriangulation flameSensor;
ServoControl servoControl(
    SERVO_PIN, SCAN_MIN_ANGLE, SCAN_MAX_ANGLE, SCAN_STEP, SCAN_DELAY, TRACKING_SPEED);
PumpControl pumpControl(
    PUMP_RELAY_PIN, PUMP_ANGLE_THRESHOLD, PUMP_PULSE_DURATION, PUMP_PULSE_DELAY);
AmbientMonitor ambientMonitor(AMBIENT_CHECK_INTERVAL);
LCDManager lcdManager(LCD_REFRESH_INTERVAL);

// Function prototypes
void performCalibration();

void setup() {
  Serial.begin(9600);
  initializeLCD();
  playStartupSequence();
  pinMode(SENSOR1_PIN, INPUT);
  pinMode(SENSOR2_PIN, INPUT);
  pinMode(SENSOR3_PIN, INPUT);
  pinMode(LED_STATUS, OUTPUT);
  pinMode(CALIBRATION_BUTTON, INPUT_PULLUP);
  initializeBuzzer();
  servoControl.begin(90);
  pumpControl.begin();
  Serial.println(F("Fire Detection Triangulation System"));
  Serial.println(F("----------------------------------"));
  Serial.println(F("Performing initial calibration..."));
  displayCalibrationMessage();
  performCalibration();
}

void loop() {
  // Calibration button logic (not a subsystem)
  if (digitalRead(CALIBRATION_BUTTON) == LOW) {
    Serial.println(F("Recalibration requested..."));
    digitalWrite(LED_STATUS, HIGH);
    displayCalibrationMessage();
    delay(500);
    performCalibration();
    digitalWrite(LED_STATUS, LOW);
  }

  // Read sensors and update flame triangulation
  int reading1 = analogRead(SENSOR1_PIN);
  int reading2 = analogRead(SENSOR2_PIN);
  int reading3 = analogRead(SENSOR3_PIN);
  flameSensor.updateReadings(reading1, reading2, reading3);
  bool flameDetected = flameSensor.isFlameDetected();
  float angle = flameDetected ? flameSensor.getFlameAngle() : 0;

  // Subsystem updates (all handle their own timing)
  ambientMonitor.update(flameSensor);
  servoControl.update(flameDetected, angle);
  pumpControl.update(flameDetected, servoControl.getCurrentAngle(), servoControl.getTargetAngle());
  lcdManager.update(flameDetected, angle, flameSensor);
  updateBuzzer(flameDetected);

  // Debug info (keep as is, or move to a DebugManager if desired)
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime >= 1000) {
    flameSensor.printDebugInfo();
    lastDebugTime = millis();
    Serial.print(F("Pump Status: "));
    Serial.println(pumpControl.isPumpActive() ? F("ON") : F("OFF"));
  }

  updateLCDDisplay();
  delay(flameDetected ? 1 : 50);
}

// Perform calibration without flame presence
void performCalibration() {
  Serial.println(F("Calibrating - ensure no flame is present"));
  
  // Play calibration tone
  playCalibrationTone();

  delay(1000); // Give time to remove flame sources
  
  // Take multiple readings and average
  int sum1 = 0, sum2 = 0, sum3 = 0;
  const int samples = 20;
  
  for (int i = 0; i < samples; i++) {
    sum1 += analogRead(SENSOR1_PIN);
    sum2 += analogRead(SENSOR2_PIN);
    sum3 += analogRead(SENSOR3_PIN);
    delay(100);
  }
  
  // Set calibration values
  flameSensor.calibrate(sum1/samples, sum2/samples, sum3/samples);
  
  Serial.println(F("Calibration complete"));
  Serial.println();
  
  // Play calibration finished tone
  playCalibrationFinishedTone();
}