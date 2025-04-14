#include "LCD.h"

// Create LCD object
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

// LCD display state tracking
struct LCDState {
  char buffer[LCD_ROWS][LCD_COLS + 1]; // Current buffer (+1 for null terminator)
  char display[LCD_ROWS][LCD_COLS + 1]; // Currently displayed content
  bool needsUpdate;                     // Flag to indicate content has changed
  bool forceUpdate;                     // Flag to force update regardless of timing
  unsigned long lastUpdateTime;         // Last time the LCD was physically updated
  unsigned long updateInterval;         // Minimum time between updates (in ms)
};

// Global state
LCDState lcdState;

// Variables for scrolling text
unsigned long previousMillis = 0;
int scrollPosition = 0;
String currentScrollingText = "";
int currentScrollingRow = 0;
int scrollDelay = 300; // Default scroll delay in ms

// Calibration warning display state
unsigned long lastDisplayToggle = 0;
int displayState = 0; // 0: normal, 1: warning, 2: comparison

/**
 * Initialize the LCD and buffering system 
 */
void initializeLCD() {
  // Initialize LCD hardware
  Wire.begin();
  lcd.init();
  lcd.backlight();
  
  // Initialize state
  for (int row = 0; row < LCD_ROWS; row++) {
    for (int col = 0; col < LCD_COLS; col++) {
      lcdState.buffer[row][col] = ' ';
      lcdState.display[row][col] = ' ';
    }
    lcdState.buffer[row][LCD_COLS] = '\0';
    lcdState.display[row][LCD_COLS] = '\0';
  }
  
  lcdState.needsUpdate = false;
  lcdState.forceUpdate = false;
  lcdState.lastUpdateTime = 0;
  lcdState.updateInterval = 1000 / LCD_UPDATE_RATE; // Set update interval from define
  
  // Show startup message (force immediate display)
  showStartupMessage();
}

/**
 * Internal: Write a character to the buffer
 */
void bufferWrite(int row, int col, char c) {
  if (row >= 0 && row < LCD_ROWS && col >= 0 && col < LCD_COLS) {
    if (lcdState.buffer[row][col] != c) {
      lcdState.buffer[row][col] = c;
      lcdState.needsUpdate = true;
    }
  }
}

/**
 * Internal: Write a string to the buffer
 */
void bufferPrint(int row, int col, const char* str) {
  while (*str && col < LCD_COLS) {
    bufferWrite(row, col++, *str++);
  }
}

/**
 * Internal: Write a string to the buffer
 */
void bufferPrint(int row, int col, const String& str) {
  for (unsigned int i = 0; i < str.length() && col < LCD_COLS; i++) {
    bufferWrite(row, col++, str[i]);
  }
}

/**
 * Clear the LCD buffer
 */
void clearLCDBuffer() {
  for (int row = 0; row < LCD_ROWS; row++) {
    for (int col = 0; col < LCD_COLS; col++) {
      lcdState.buffer[row][col] = ' ';
    }
    lcdState.buffer[row][LCD_COLS] = '\0';
  }
  lcdState.needsUpdate = true;
}

/**
 * Main LCD update function - call this regularly in main loop
 */
void updateLCDDisplay() {
  unsigned long currentTime = millis();
  
  // Check if update is needed and timing constraints are met
  if ((lcdState.needsUpdate || lcdState.forceUpdate) && 
      (lcdState.forceUpdate || currentTime - lcdState.lastUpdateTime >= lcdState.updateInterval)) {
    
    // Update LCD with buffered content
    bool anyChanges = false;
    
    for (int row = 0; row < LCD_ROWS; row++) {
      // Check if this row has changed
      bool rowChanged = false;
      for (int col = 0; col < LCD_COLS; col++) {
        if (lcdState.buffer[row][col] != lcdState.display[row][col]) {
          rowChanged = true;
          break;
        }
      }
      
      // If row changed, update the entire row
      if (rowChanged) {
        anyChanges = true;
        lcd.setCursor(0, row);
        for (int col = 0; col < LCD_COLS; col++) {
          lcdState.display[row][col] = lcdState.buffer[row][col];
          lcd.write(lcdState.display[row][col]);
        }
      }
    }
    
    // Update state
    lcdState.lastUpdateTime = currentTime;
    lcdState.needsUpdate = false;
    lcdState.forceUpdate = false;
  }
}

/**
 * Display the startup message
 */
void showStartupMessage() {
  clearLCDBuffer();
  bufferPrint(0, 0, "   Fire System   ");
  bufferPrint(1, 0, " Initializing... ");
  lcdState.forceUpdate = true;
  updateLCDDisplay();
  delay(2000);
}

/**
 * Update the LCD buffer with flame detection information
 */
void updateLCD(bool flameDetected, float angle) {
  clearLCDBuffer();
  
  // First row: Status message
  if (flameDetected) {
    bufferPrint(0, 0, "FIRE DETECTED!");
  } else {
    bufferPrint(0, 0, "Monitoring...");
  }
  
  // Second row: Angle information
  if (flameDetected) {
    // Convert angle to string with desired precision
    char angleStr[7];  // Enough for -30.0 plus null terminator
    dtostrf(angle, 4, 1, angleStr);  // 4 chars total, 1 after decimal
    
    // Trim leading spaces (dtostrf pads with spaces)
    char* anglePtr = angleStr;
    while (*anglePtr == ' ') anglePtr++;
    
    // Buffer angle display
    bufferPrint(1, 0, "Angle: ");
    bufferPrint(1, 7, anglePtr);
    bufferPrint(1, 7 + strlen(anglePtr), " deg");
  } else {
    bufferPrint(1, 0, "No threat");
  }
}

/**
 * Display calibration message during system calibration
 */
void displayCalibrationMessage() {
  clearLCDBuffer();
  bufferPrint(0, 0, "Calibrating...");
  bufferPrint(1, 0, "Please wait");
  lcdState.forceUpdate = true;
  updateLCDDisplay();
}

/**
 * Display calibration warning message
 */
void displayCalibrationWarning() {
  clearLCDBuffer();
  bufferPrint(0, 0, "RECALIBRATION");
  bufferPrint(1, 0, "RECOMMENDED!");
}

/**
 * Display calibration comparison (saved vs current)
 */
void displayCalibrationCompare(int saved1, int saved2, int saved3, 
                              float current1, float current2, float current3) {
  clearLCDBuffer();
  
  // Find saved value with most deviation
  int savedAvg = (saved1 + saved2 + saved3) / 3;
  int maxSavedDev = 0;
  int maxSavedVal = saved1;
  
  // Check each saved value manually
  int dev = abs(saved1 - savedAvg);
  if (dev > maxSavedDev) {
    maxSavedDev = dev;
    maxSavedVal = saved1;
  }
  
  dev = abs(saved2 - savedAvg);
  if (dev > maxSavedDev) {
    maxSavedDev = dev;
    maxSavedVal = saved2;
  }
  
  dev = abs(saved3 - savedAvg);
  if (dev > maxSavedDev) {
    maxSavedDev = dev;
    maxSavedVal = saved3;
  }
  
  // Find current value with most deviation
  float currentAvg = (current1 + current2 + current3) / 3.0;
  float maxCurrentDev = 0;
  float maxCurrentVal = current1;
  
  // Check each current value manually
  float devF = abs(current1 - currentAvg);
  if (devF > maxCurrentDev) {
    maxCurrentDev = devF;
    maxCurrentVal = current1;
  }
  
  devF = abs(current2 - currentAvg);
  if (devF > maxCurrentDev) {
    maxCurrentDev = devF;
    maxCurrentVal = current2;
  }
  
  devF = abs(current3 - currentAvg);
  if (devF > maxCurrentDev) {
    maxCurrentDev = devF;
    maxCurrentVal = current3;
  }
  
  // Display values with most deviation
  char valStr[10];
  
  bufferPrint(0, 0, "Saved:   ");
  sprintf(valStr, "%d", maxSavedVal);
  bufferPrint(0, 9, valStr);
  
  bufferPrint(1, 0, "Current: ");
  sprintf(valStr, "%d", (int)maxCurrentVal);
  bufferPrint(1, 9, valStr);
}

/**
 * Update LCD with calibration status (handles cycling between displays)
 */
void updateLCDWithCalibrationStatus(bool flameDetected, float angle, bool calibrationNeeded, 
                                   int savedAmbient1, int savedAmbient2, int savedAmbient3,
                                   float currentAmbient1, float currentAmbient2, float currentAmbient3) {
  // If calibration not needed, just show normal display
  if (!calibrationNeeded) {
    updateLCD(flameDetected, angle);
    displayState = 0;
    return;
  }
  
  // Handle display rotation for calibration warnings
  unsigned long currentMillis = millis();
  
  // Rotate displays every 3 seconds
  if (currentMillis - lastDisplayToggle >= 3000) {
    lastDisplayToggle = currentMillis;
    
    // Cycle through display states
    displayState = (displayState + 1) % 3;
    
    switch (displayState) {
      case 0:
        // Normal display
        updateLCD(flameDetected, angle);
        break;
      case 1:
        // Warning display
        displayCalibrationWarning();
        break;
      case 2:
        // Comparison display
        displayCalibrationCompare(savedAmbient1, savedAmbient2, savedAmbient3,
                                currentAmbient1, currentAmbient2, currentAmbient3);
        break;
    }
  }
}

/**
 * Clear the LCD display
 */
void clearLCD() {
  clearLCDBuffer();
  lcdState.forceUpdate = true;
  updateLCDDisplay();
}

/**
 * Scroll long text on a specific row - non-blocking implementation
 * Must be called repeatedly in the main loop
 */
void scrollLongText(const String& text, int row, int delayMs) {
  // Only restart scrolling if text or row has changed
  if (text != currentScrollingText || row != currentScrollingRow) {
    currentScrollingText = text;
    currentScrollingRow = row;
    scrollPosition = 0;
    scrollDelay = delayMs;
  }
  
  unsigned long currentMillis = millis();
  
  // Check if it's time to scroll
  if (currentMillis - previousMillis >= (unsigned long)scrollDelay) {
    previousMillis = currentMillis;
    
    // If text fits on the display, no scrolling needed
    if (text.length() <= LCD_COLS) {
      clearLCDBuffer();
      bufferPrint(row, 0, text);
      // Add spaces to clear the rest of the line is handled by clearLCDBuffer
      return;
    }
    
    // Handle scrolling for text longer than the display
    int textLength = text.length();
    String displayText;
    
    // Create the text to display (with wrap-around)
    if (scrollPosition < textLength) {
      displayText = text.substring(scrollPosition);
      if (displayText.length() < LCD_COLS) {
        displayText += " · " + text.substring(0, LCD_COLS - displayText.length() - 3);
      }
    } else {
      scrollPosition = 0;
      displayText = text.substring(0, LCD_COLS);
    }
    
    // Write to buffer
    for (int col = 0; col < LCD_COLS; col++) {
      if (col < (int)displayText.length()) {
        bufferWrite(row, col, displayText[col]);
      } else {
        bufferWrite(row, col, ' ');
      }
    }
    
    // Increment scroll position
    scrollPosition++;
    if (scrollPosition > textLength + 3) {
      scrollPosition = 0; // Reset with a small pause at the beginning
    }
  }
} 