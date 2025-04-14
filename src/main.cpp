#include <Arduino.h>
#include <Servo.h>
#include "../include/FlameTriangulation.h"
#include "../include/Buzzer.h"
#include "../include/LCD.h"

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
Servo trackingServo;
int currentServoAngle = 90;  // Start at center position
bool scanDirection = true;   // true = increasing angle, false = decreasing

// LCD state tracking
unsigned long lastLCDUpdate = 0;
bool lastFlameState = false;
float lastAngle = 0;

// Ambient monitoring
unsigned long lastAmbientCheck = 0;

// Pump control state
bool pumpEnabled = false;
bool pumpActive = false;
unsigned long pumpStateChangeTime = 0;

// Function prototypes
void performCalibration();
void updateServoPosition(float flameAngle, bool flameDetected);
int lerpAngle(int current, int target, float factor);
void updateLCDIfNeeded(bool flameDetected, float angle);
void checkAmbientDrift();
void updatePumpState(bool flameDetected, int servoAngle, int targetServoAngle);

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Initialize LCD
  initializeLCD();
  
  // Play startup sequence
  playStartupSequence();
  
  // Configure pins
  pinMode(SENSOR1_PIN, INPUT);
  pinMode(SENSOR2_PIN, INPUT);
  pinMode(SENSOR3_PIN, INPUT);
  pinMode(LED_STATUS, OUTPUT);
  pinMode(CALIBRATION_BUTTON, INPUT_PULLUP);
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  
  // Ensure pump is off at startup
  digitalWrite(PUMP_RELAY_PIN, HIGH);
  
  // Initialize buzzer
  initializeBuzzer();
  
  // Initialize servo
  trackingServo.attach(SERVO_PIN);
  trackingServo.write(currentServoAngle);  // Center the servo
  
  // Initial calibration
  Serial.println(F("Fire Detection Triangulation System"));
  Serial.println(F("----------------------------------"));
  Serial.println(F("Performing initial calibration..."));
  displayCalibrationMessage();
  performCalibration();
}

void loop() {
  // Check for calibration button press
  if (digitalRead(CALIBRATION_BUTTON) == LOW) {
    Serial.println(F("Recalibration requested..."));
    digitalWrite(LED_STATUS, HIGH);
    displayCalibrationMessage();
    delay(500);
    performCalibration();
    digitalWrite(LED_STATUS, LOW);
  }
  
  // Read sensor values
  int reading1 = analogRead(SENSOR1_PIN);
  int reading2 = analogRead(SENSOR2_PIN);
  int reading3 = analogRead(SENSOR3_PIN);
  
  // Update flame triangulation with new readings
  flameSensor.updateReadings(reading1, reading2, reading3);
  
  // Check if flame is detected
  bool flameDetected = flameSensor.isFlameDetected();
  
  // Periodically check for ambient drift when no flame is detected
  if (!flameDetected) {
    checkAmbientDrift();
  }
  
  // Variables for pump control
  int targetServoAngle = 90; // Default center position
  
  if (flameDetected) {
    // Get flame angle and confidence
    float angle = flameSensor.getFlameAngle();
    float confidence = flameSensor.getConfidence();
    
    // Calculate target servo angle for fire
    targetServoAngle = map(constrain(angle, -30, 30), -30, 30, 150, 30);
    
    // Update LCD with appropriate display based on calibration status
    if (flameSensor.calibrationNeeded) {
      updateLCDWithCalibrationStatus(
        flameDetected, angle, true,
        flameSensor.ambientLevel1, flameSensor.ambientLevel2, flameSensor.ambientLevel3,
        flameSensor.getCurrentAmbient1(), flameSensor.getCurrentAmbient2(), flameSensor.getCurrentAmbient3()
      );
    } else {
      updateLCDIfNeeded(flameDetected, angle);
    }
    
    // Display results on serial monitor
    Serial.print(F("FLAME DETECTED: "));
    Serial.print(angle, 1);
    Serial.print(F("° (confidence: "));
    Serial.print(confidence * 100, 0);
    Serial.println(F("%)"));
    
    // Visual indicator
    digitalWrite(LED_STATUS, HIGH);
    
    // Update servo to point at flame
    updateServoPosition(angle, true);
  } else {
    // Update LCD with appropriate display based on calibration status
    if (flameSensor.calibrationNeeded) {
      updateLCDWithCalibrationStatus(
        flameDetected, 0, true,
        flameSensor.ambientLevel1, flameSensor.ambientLevel2, flameSensor.ambientLevel3,
        flameSensor.getCurrentAmbient1(), flameSensor.getCurrentAmbient2(), flameSensor.getCurrentAmbient3()
      );
    } else {
      updateLCDIfNeeded(flameDetected, 0);
    }
    
    digitalWrite(LED_STATUS, LOW);
    
    // Update servo to perform scanning motion
    updateServoPosition(0, false);
  }
  
  // Update pump control based on flame detection and servo position
  updatePumpState(flameDetected, currentServoAngle, targetServoAngle);
  
  // Update buzzer
  updateBuzzer(flameDetected);
  
  // Print debug information every second
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime >= 1000) {
    flameSensor.printDebugInfo();
    lastDebugTime = millis();
    
    // Add pump status to debug output
    Serial.print(F("Pump Status: "));
    Serial.println(pumpActive ? F("ON") : F("OFF"));
  }
  
  // Update the LCD display
  updateLCDDisplay();
  
  // Brief delay
  delay(flameDetected ? 1 : 50);
}

// Update pump state based on flame detection and servo position
void updatePumpState(bool flameDetected, int servoAngle, int targetServoAngle) {
  // Determine if pump should be enabled based on flame detection and servo alignment
  bool shouldEnable = false;
  
  if (flameDetected) {
    // Calculate how close the servo is to the target angle
    int angleDifference = abs(servoAngle - targetServoAngle);
    
    // Enable pump if servo is pointing within threshold of the target
    shouldEnable = (angleDifference <= PUMP_ANGLE_THRESHOLD);
    
    if (shouldEnable) {
      // Debug output when pump first becomes enabled
      if (!pumpEnabled) {
        Serial.println(F("Pump enabled - servo aligned with fire"));
      }
    }
  }
  
  // Update pump enabled state
  pumpEnabled = shouldEnable;
  
  // Handle pulsing behavior when pump is enabled
  if (pumpEnabled) {
    unsigned long currentTime = millis();
    
    // Check if it's time to change the pump state
    if (currentTime - pumpStateChangeTime >= (pumpActive ? PUMP_PULSE_DURATION : PUMP_PULSE_DELAY)) {
      // Toggle pump state
      pumpActive = !pumpActive;
      
      // Update the relay (LOW to activate, HIGH to deactivate - active low relay)
      digitalWrite(PUMP_RELAY_PIN, pumpActive ? LOW : HIGH);
      
      // Update the time of the state change
      pumpStateChangeTime = currentTime;
    }
  } else {
    // Ensure pump is off when not enabled
    if (pumpActive) {
      pumpActive = false;
      digitalWrite(PUMP_RELAY_PIN, HIGH);
    }
  }
}

// Check for ambient drift and update calibration status
void checkAmbientDrift() {
  unsigned long currentMillis = millis();
  
  // Only check periodically to avoid excessive processing
  if (currentMillis - lastAmbientCheck >= AMBIENT_CHECK_INTERVAL) {
    lastAmbientCheck = currentMillis;
    
    // Update the calibration monitoring
    flameSensor.updateCalibrationMonitoring();
    
    // If calibration is needed and this is the first time it's been detected,
    // play the warning tone
    if (flameSensor.calibrationNeeded && !flameSensor.calibrationWarningTriggered) {
      flameSensor.calibrationWarningTriggered = true;
      playCalibrationWarningTone();
      
      // Output to serial for debugging
      Serial.println(F("CALIBRATION WARNING: Ambient drift detected!"));
    }
  }
}

// Wrapper for LCD updates that limits refresh rate
void updateLCDIfNeeded(bool flameDetected, float angle) {
  unsigned long currentMillis = millis();
  
  // Update LCD only if enough time has passed since last update
  // or if detection status has changed
  // or if angle has changed significantly
  if (currentMillis - lastLCDUpdate >= LCD_REFRESH_INTERVAL ||
      flameDetected != lastFlameState ||
      (flameDetected && abs(angle - lastAngle) > 3.0)) {
    
    updateLCD(flameDetected, angle);
    
    // Update state tracking variables
    lastLCDUpdate = currentMillis;
    lastFlameState = flameDetected;
    lastAngle = angle;
  }
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

// Linear interpolation function for smoother servo movement
int lerpAngle(int current, int target, float factor) {
  // Constrain the lerp factor between 0 and 1
  factor = constrain(factor, 0.0, 1.0);
  
  // Calculate the interpolated value
  float result = current + factor * (target - current);
  
  // Return the rounded integer value
  return round(result);
}

// Update servo position based on detection status
void updateServoPosition(float flameAngle, bool flameDetected) {
  static unsigned long lastServoUpdate = 0;
  static unsigned long currentMillis = millis();
  
  // Determine if it's time to update the servo position
  currentMillis = millis();
  if (currentMillis - lastServoUpdate < SCAN_DELAY && !flameDetected) {
    // Not enough time has passed for an update in scanning mode
    return;
  }
  
  // Update the last servo update time if we're going to move the servo
  lastServoUpdate = currentMillis;
  
  if (flameDetected) {
    // Convert flame angle to servo angle
    // Flame angle is from -30 to +30 degrees (relative to front)
    // Servo angle is from 0 to 180 degrees (hardware positions)
    
    // Map flame angle range (-30 to +30) to servo range (30 to 150)
    // This gives the servo a 120-degree range of motion centered at 90 degrees
    int targetAngle = map(constrain(flameAngle, -30, 30), -30, 30, 150, 30);
    
    // Use linear interpolation for smooth movement
    currentServoAngle = lerpAngle(currentServoAngle, targetAngle, TRACKING_SPEED);
    
    // Update servo position
    trackingServo.write(currentServoAngle);
    
    // Print tracking info (optional, for debugging)
    // Serial.print("Target: "); Serial.print(targetAngle);
    // Serial.print(", Current: "); Serial.println(currentServoAngle);
  } else {
    // Perform scanning motion
    // Check if we need to change direction
    if (scanDirection) {
      // Moving towards maximum
      currentServoAngle += SCAN_STEP;
      if (currentServoAngle >= SCAN_MAX_ANGLE) {
        currentServoAngle = SCAN_MAX_ANGLE;
        scanDirection = false;  // Start moving backwards
      }
    } else {
      // Moving towards minimum
      currentServoAngle -= SCAN_STEP;
      if (currentServoAngle <= SCAN_MIN_ANGLE) {
        currentServoAngle = SCAN_MIN_ANGLE;
        scanDirection = true;  // Start moving forwards
      }
    }
    
    // Update servo position
    trackingServo.write(currentServoAngle);
    // No delay needed here since we use millis() timing
  }
}