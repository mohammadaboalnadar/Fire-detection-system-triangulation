/**
 * Pump Control Demo for the Fire Detection System
 * 
 * This example demonstrates the pump control functionality:
 * - When a fire is detected, the servo moves to point at it
 * - When the servo is correctly aligned (within threshold), the pump activates
 * - The pump works in pulses rather than continuously
 * 
 * Adjustable parameters:
 * - PUMP_ANGLE_THRESHOLD: How close servo must be to target angle (in degrees)
 * - PUMP_PULSE_DURATION: How long each water pulse lasts
 * - PUMP_PULSE_DELAY: How long between pulses
 */

#include <Arduino.h>
#include <Servo.h>

// Pin definitions
#define FLAME_SENSOR_PIN A0
#define SERVO_PIN 9
#define PUMP_RELAY_PIN 6
#define LED_PIN 13

// Servo parameters
#define SERVO_MIN_ANGLE 30
#define SERVO_MAX_ANGLE 150
#define SERVO_CENTER 90

// Pump control parameters
#define PUMP_ANGLE_THRESHOLD 3    // Activate pump when within +/- degrees of target
#define PUMP_PULSE_DURATION 500   // Duration of water pulse in milliseconds
#define PUMP_PULSE_DELAY 1000     // Delay between pulses in milliseconds

// Global objects
Servo trackingServo;

// State variables
int currentServoAngle = SERVO_CENTER;
int targetServoAngle = SERVO_CENTER;
bool flameDetected = false;
bool pumpEnabled = false;
bool pumpActive = false;
unsigned long pumpStateChangeTime = 0;

// Simulate flame detection with random values
unsigned long lastFlameUpdate = 0;
const unsigned long flameUpdateInterval = 5000; // Update "flame" every 5 seconds

void setup() {
  // Initialize serial
  Serial.begin(9600);
  Serial.println(F("Pump Control Demo"));
  Serial.println(F("------------------"));
  
  // Initialize pins
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  
  // Make sure pump is off initially
  digitalWrite(PUMP_RELAY_PIN, LOW);
  
  // Initialize servo
  trackingServo.attach(SERVO_PIN);
  trackingServo.write(SERVO_CENTER);
  
  Serial.println(F("System initialized"));
  Serial.println(F("Parameters:"));
  Serial.print(F("- Angle threshold: ±"));
  Serial.print(PUMP_ANGLE_THRESHOLD);
  Serial.println(F(" degrees"));
  Serial.print(F("- Pulse duration: "));
  Serial.print(PUMP_PULSE_DURATION);
  Serial.println(F(" ms"));
  Serial.print(F("- Pulse delay: "));
  Serial.print(PUMP_PULSE_DELAY);
  Serial.println(F(" ms"));
  Serial.println();
}

void loop() {
  // Simulate flame detection
  simulateFlameDetection();
  
  // Move servo towards target angle with smooth motion
  updateServoPosition();
  
  // Update pump state based on servo position
  updatePumpState();
  
  // Debug output
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime >= 500) {
    lastDebugTime = millis();
    printStatus();
  }
  
  // Brief delay
  delay(20);
}

// Simulate a flame being detected at random positions
void simulateFlameDetection() {
  unsigned long currentTime = millis();
  
  // Periodically update flame status
  if (currentTime - lastFlameUpdate >= flameUpdateInterval) {
    lastFlameUpdate = currentTime;
    
    // 75% chance of detecting flame
    flameDetected = (random(100) < 75);
    
    if (flameDetected) {
      // Random target angle within servo range
      targetServoAngle = random(SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
      
      Serial.print(F("Flame detected at angle: "));
      Serial.println(targetServoAngle);
    } else {
      Serial.println(F("No flame detected"));
      
      // Return to center position when no flame
      targetServoAngle = SERVO_CENTER;
    }
  }
  
  // Update LED to indicate flame detection
  digitalWrite(LED_PIN, flameDetected ? HIGH : LOW);
}

// Update servo position with smooth motion
void updateServoPosition() {
  // Simple linear interpolation for smooth movement
  if (currentServoAngle != targetServoAngle) {
    // Move towards target by 1 degree per loop
    if (currentServoAngle < targetServoAngle) {
      currentServoAngle++;
    } else {
      currentServoAngle--;
    }
    
    // Apply servo position
    trackingServo.write(currentServoAngle);
  }
}

// Update pump state based on flame detection and servo position
void updatePumpState() {
  // Determine if pump should be enabled based on flame detection and servo alignment
  bool shouldEnable = false;
  
  if (flameDetected) {
    // Calculate how close the servo is to the target angle
    int angleDifference = abs(currentServoAngle - targetServoAngle);
    
    // Enable pump if servo is pointing within threshold of the target
    shouldEnable = (angleDifference <= PUMP_ANGLE_THRESHOLD);
  }
  
  // Update pump enabled state
  if (shouldEnable != pumpEnabled) {
    pumpEnabled = shouldEnable;
    
    if (pumpEnabled) {
      Serial.println(F("Pump enabled - servo aligned with target"));
    } else {
      Serial.println(F("Pump disabled"));
    }
  }
  
  // Handle pulsing behavior when pump is enabled
  if (pumpEnabled) {
    unsigned long currentTime = millis();
    
    // Check if it's time to change the pump state
    if (currentTime - pumpStateChangeTime >= (pumpActive ? PUMP_PULSE_DURATION : PUMP_PULSE_DELAY)) {
      // Toggle pump state
      pumpActive = !pumpActive;
      
      // Update the relay
      digitalWrite(PUMP_RELAY_PIN, pumpActive ? HIGH : LOW);
      
      // Debug output
      if (pumpActive) {
        Serial.println(F("Pump ON"));
      } else {
        Serial.println(F("Pump OFF"));
      }
      
      // Update the time of the state change
      pumpStateChangeTime = currentTime;
    }
  } else {
    // Ensure pump is off when not enabled
    if (pumpActive) {
      pumpActive = false;
      digitalWrite(PUMP_RELAY_PIN, LOW);
      Serial.println(F("Pump forced OFF"));
    }
  }
}

// Print system status
void printStatus() {
  Serial.print(F("Servo: "));
  Serial.print(currentServoAngle);
  Serial.print(F("° → "));
  Serial.print(targetServoAngle);
  Serial.print(F("° | Diff: "));
  Serial.print(abs(currentServoAngle - targetServoAngle));
  
  if (flameDetected) {
    Serial.print(F(" | FLAME"));
  } else {
    Serial.print(F(" | ----"));
  }
  
  Serial.print(F(" | Pump: "));
  if (pumpEnabled) {
    Serial.print(F("Enabled"));
    Serial.print(pumpActive ? F("-ON") : F("-off"));
  } else {
    Serial.print(F("Disabled"));
  }
  
  Serial.println();
} 