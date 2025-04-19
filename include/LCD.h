#ifndef LCD_H
#define LCD_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// LCD parameters
#define LCD_I2C_ADDR 0x27  // Default I2C address for most 16x2 LCD modules (may need adjustment)
#define LCD_COLS 16
#define LCD_ROWS 2
#define LCD_UPDATE_RATE 15 // LCD refresh rate in Hz (max 30Hz)

// DHT Sensor parameters
#define DHTPIN 3           // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11      // DHT 11
#define DHT_READ_INTERVAL 2000 // Time between temperature readings (2 seconds)
#define DHT_DISPLAY_TOGGLE_INTERVAL 5000 // Time to alternate between temp and humidity display (5 seconds)

// External LCD object declaration
extern LiquidCrystal_I2C lcd;
extern DHT dht;

// External pump status variables
extern bool pumpEnabled;
extern bool pumpActive;

// Function prototypes
void initializeLCD();
void updateLCDDisplay(); // Main update function - call this from loop()
void clearLCDBuffer();
void updateLCD(bool flameDetected, float angle);
void displayCalibrationMessage();
void clearLCD();
void showStartupMessage();
void scrollLongText(const String& text, int row, int delay_ms);

// DHT sensor functions
void initializeDHT();
void updateDHTReadings();
float getTemperature();
float getHumidity();
void updateLCDWithTempHumidity(bool flameDetected, float angle);

// Calibration warning display
void displayCalibrationWarning();
void displayCalibrationCompare(int saved1, int saved2, int saved3, 
                              float current1, float current2, float current3);
void updateLCDWithCalibrationStatus(bool flameDetected, float angle, bool calibrationNeeded, 
                                   int savedAmbient1, int savedAmbient2, int savedAmbient3,
                                   float currentAmbient1, float currentAmbient2, float currentAmbient3);

#endif // LCD_H