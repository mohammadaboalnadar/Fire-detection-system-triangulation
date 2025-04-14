/*
 * PumpControl_LCD_Demo
 * 
 * This example demonstrates the pump control functionality with LCD display
 * for a fire detection system.
 * 
 * The sketch simulates flame detection, moves a servo to point at the flame,
 * controls a water pump with pulsed operation, and displays status on an LCD.
 */

#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "../include/LCD.h" // Using the project's LCD implementation

// Pin definitions
const int FLAME_SENSOR_PIN = 2;
const int SERVO_PIN = 9;
const int PUMP_RELAY_PIN = 6;
const int LED_PIN = 13;

// Servo configuration
Servo directionServo;
int currentAngle = 90;
int targetAngle = 90;
const int SERVO_STEP = 5;

// Pump control parameters (adjustable)
const int PUMP_ANGLE_THRESHOLD = 3;  // Degrees from target to activate pump
const long PUMP_PULSE_DURATION = 500; // Milliseconds for each pulse
const long PUMP_PULSE_DELAY = 1000;  // Milliseconds between pulses

// State variables
bool flameDetected = false;
bool pumpEnabled = false;
bool pumpState = false;
unsigned long lastPumpToggleTime = 0;
unsigned long lastFlameCheckTime = 0;
const long FLAME_CHECK_INTERVAL = 5000; // Check for flames every 5 seconds

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Initialize pins
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize servo
  directionServo.attach(SERVO_PIN);
  directionServo.write(currentAngle);
  
  // Initialize LCD
  initializeLCD();
  showStartupMessage();
  
  // Initial display update
  updateSystemStatus();
  
  Serial.println("System initialized");
}

void loop() {
  // Periodically simulate flame detection
  if (millis() - lastFlameCheckTime > FLAME_CHECK_INTERVAL) {
    simulateFlameDetection();
    lastFlameCheckTime = millis();
  }
  
  // Update servo position
  updateServoPosition();
  
  // Update pump state
  updatePumpState();
  
  // Update LCD display
  updateSystemStatus();
  
  delay(100); // Short delay for stability
}

void simulateFlameDetection() {
  // Randomly detect flame for demonstration purposes
  flameDetected = random(0, 10) > 3; // 60% chance of flame detection
  
  if (flameDetected) {
    // Simulate a flame detected at a random angle
    targetAngle = random(0, 180);
    digitalWrite(LED_PIN, HIGH);
    Serial.print("Flame detected at angle: ");
    Serial.println(targetAngle);
  } else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("No flame detected");
  }
}

void updateServoPosition() {
  // Move servo toward target angle
  if (currentAngle < targetAngle) {
    currentAngle += SERVO_STEP;
    if (currentAngle > targetAngle) {
      currentAngle = targetAngle;
    }
  } else if (currentAngle > targetAngle) {
    currentAngle -= SERVO_STEP;
    if (currentAngle < targetAngle) {
      currentAngle = targetAngle;
    }
  }
  
  directionServo.write(currentAngle);
  
  // Update pump enabled status based on servo position
  pumpEnabled = flameDetected && 
    abs(currentAngle - targetAngle) <= PUMP_ANGLE_THRESHOLD;
  
  Serial.print("Servo position: ");
  Serial.print(currentAngle);
  Serial.print(", Target: ");
  Serial.print(targetAngle);
  Serial.print(", Pump enabled: ");
  Serial.println(pumpEnabled ? "YES" : "NO");
}

void updatePumpState() {
  if (pumpEnabled) {
    // Implement pulsing behavior when pump is enabled
    unsigned long currentTime = millis();
    
    if (currentTime - lastPumpToggleTime >= 
        (pumpState ? PUMP_PULSE_DURATION : PUMP_PULSE_DELAY)) {
      // Toggle pump state
      pumpState = !pumpState;
      lastPumpToggleTime = currentTime;
      
      // Set relay accordingly (LOW to activate, HIGH to deactivate)
      digitalWrite(PUMP_RELAY_PIN, pumpState ? LOW : HIGH);
      
      Serial.print("Pump switched ");
      Serial.println(pumpState ? "ON" : "OFF");
    }
  } else {
    // Make sure pump is off when not enabled
    if (pumpState) {
      pumpState = false;
      digitalWrite(PUMP_RELAY_PIN, HIGH);
      Serial.println("Pump forced OFF");
    }
  }
}

void updateSystemStatus() {
  // Clear LCD and update with current system status
  clearLCD();
  
  // Line 1: Flame status and target angle
  lcd.setCursor(0, 0);
  if (flameDetected) {
    lcd.print("Fire @ ");
    lcd.print(targetAngle);
    lcd.print(" deg");
  } else {
    lcd.print("No Fire Detected");
  }
  
  // Line 2: Servo position and pump status
  lcd.setCursor(0, 1);
  lcd.print("Servo:");
  lcd.print(currentAngle);
  lcd.print(" Pump:");
  
  if (pumpEnabled) {
    lcd.print(pumpState ? "ON" : "...");
  } else {
    lcd.print("OFF");
  }
} 