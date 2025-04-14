#include "../include/FlameTriangulation.h"

FlameTriangulation::FlameTriangulation() {
  // Initialize ambient levels
  ambientLevel1 = 1023;
  ambientLevel2 = 1023;
  ambientLevel3 = 1023;
  
  // Initialize readings
  rawReading1 = 0;
  rawReading2 = 0;
  rawReading3 = 0;
  processedReading1 = 0;
  processedReading2 = 0;
  processedReading3 = 0;
  
  // Initialize buffer
  bufferIndex = 0;
  for (int i = 0; i < bufferSize; i++) {
    readingBuffer1[i] = 0;
    readingBuffer2[i] = 0;
    readingBuffer3[i] = 0;
  }
  
  // Initialize ambient tracking variables
  avgAmbient1 = 1023;
  avgAmbient2 = 1023;
  avgAmbient3 = 1023;
  lastAmbientUpdate = 0;
  validSampleCount = 0;
  cooldownEndTime = 0;
  calibrationNeeded = false;
  calibrationWarningTriggered = false;
}

void FlameTriangulation::calibrate(int reading1, int reading2, int reading3) {
  // Store ambient light readings
  ambientLevel1 = reading1;
  ambientLevel2 = reading2;
  ambientLevel3 = reading3;
  
  // Reset buffers
  for (int i = 0; i < bufferSize; i++) {
    readingBuffer1[i] = reading1;
    readingBuffer2[i] = reading2;
    readingBuffer3[i] = reading3;
  }
  
  // Reset ambient tracking
  avgAmbient1 = reading1;
  avgAmbient2 = reading2;
  avgAmbient3 = reading3;
  validSampleCount = 0;
  calibrationNeeded = false;
  calibrationWarningTriggered = false;
}

void FlameTriangulation::updateReadings(int reading1, int reading2, int reading3) {
  // Store raw readings
  rawReading1 = reading1;
  rawReading2 = reading2;
  rawReading3 = reading3;
  
  // Update buffers
  updateBuffers(reading1, reading2, reading3);
  
  // Get smoothed readings
  processedReading1 = getSmoothedReading(readingBuffer1);
  processedReading2 = getSmoothedReading(readingBuffer2);
  processedReading3 = getSmoothedReading(readingBuffer3);
  
  // Update ambient tracking (pass current flame detection status)
  bool flameDetected = isFlameDetected(); 
  updateAmbientTracking(flameDetected);
}

void FlameTriangulation::updateBuffers(int r1, int r2, int r3) {
  // Add new readings to buffer
  readingBuffer1[bufferIndex] = r1;
  readingBuffer2[bufferIndex] = r2;
  readingBuffer3[bufferIndex] = r3;
  
  // Increment buffer index with wrap-around
  bufferIndex = (bufferIndex + 1) % bufferSize;
}

int FlameTriangulation::getSmoothedReading(int buffer[]) {
  // Simple moving average
  long sum = 0;
  for (int i = 0; i < bufferSize; i++) {
    sum += buffer[i];
  }
  return sum / bufferSize;
}

bool FlameTriangulation::isFlameDetected() {
  // Check if any sensor reading is significantly below ambient level
  return (
    (ambientLevel1 - processedReading1 > threshold) ||
    (ambientLevel2 - processedReading2 > threshold) ||
    (ambientLevel3 - processedReading3 > threshold)
  );
}

float FlameTriangulation::calculateRelativeIntensity(int reading, int ambient) {
  // Convert reading to relative intensity (0.0 - 1.0)
  int diff = ambient - reading;
  if (diff <= 0) return 0.0;
  
  // Cap at reasonable maximum
  const int maxDiff = 500;
  if (diff > maxDiff) diff = maxDiff;
  
  return (float)diff / maxDiff;
}

void FlameTriangulation::updateAmbientTracking(bool flameDetected) {
  // Only update ambient tracking if no flame is detected
  // and we're not in a cooldown period after flame detection
  if (!flameDetected && millis() >= cooldownEndTime) {
    // Update running average with new readings (exponential moving average)
    // Use a slow-moving average (0.95/0.05 weights) for stability
    avgAmbient1 = (avgAmbient1 * 0.95) + (processedReading1 * 0.05);
    avgAmbient2 = (avgAmbient2 * 0.95) + (processedReading2 * 0.05);
    avgAmbient3 = (avgAmbient3 * 0.95) + (processedReading3 * 0.05);
    
    // Increment valid sample counter
    if (validSampleCount < 0xFFFF) {  // Prevent overflow
      validSampleCount++;
    }
  } 
  else if (flameDetected) {
    // Set cooldown period after flame detection (3 seconds)
    cooldownEndTime = millis() + 3000;
  }
}

void FlameTriangulation::updateCalibrationMonitoring() {
  // Only check for drift after collecting enough samples
  if (validSampleCount >= MIN_SAMPLES_FOR_DRIFT) {
    // Calculate absolute deviations for each sensor
    float dev1 = abs(avgAmbient1 - ambientLevel1);
    float dev2 = abs(avgAmbient2 - ambientLevel2);
    float dev3 = abs(avgAmbient3 - ambientLevel3);
    
    // Check if any sensor has drifted beyond the warning threshold
    if (dev1 > DRIFT_WARNING_THRESHOLD || 
        dev2 > DRIFT_WARNING_THRESHOLD || 
        dev3 > DRIFT_WARNING_THRESHOLD) {
      
      calibrationNeeded = true;
    } else {
      calibrationNeeded = false;
    }
  }
}

void FlameTriangulation::resetCalibrationWarning() {
  calibrationNeeded = false;
  calibrationWarningTriggered = false;
}

float FlameTriangulation::getFlameAngle() {
  // Use different methods based on which sensors detect the flame
  bool s1Detect = (ambientLevel1 - processedReading1 > threshold);
  bool s2Detect = (ambientLevel2 - processedReading2 > threshold);
  bool s3Detect = (ambientLevel3 - processedReading3 > threshold);
  
  // If all three sensors detect the flame, use weighted triangulation
  if (s1Detect && s2Detect && s3Detect) {
    return weightedAngularTriangulation();
  }
  
  // If only two sensors detect the flame, use dual sensor method
  if ((s1Detect && s2Detect) || (s1Detect && s3Detect) || (s2Detect && s3Detect)) {
    return dualSensorEstimation();
  }
  
  // If only one sensor detects the flame, estimate direction based on that sensor
  if (s1Detect) return sensorAngleLimit;         // Right sensor
  if (s2Detect) return -sensorAngleLimit;        // Left sensor
  if (s3Detect) return 0.0;                      // Middle sensor
  
  // Fallback (shouldn't reach here if isFlameDetected() was checked first)
  return 0.0;
}

float FlameTriangulation::dualSensorEstimation() {
  // Calculate relative intensities
  float i1 = calculateRelativeIntensity(processedReading1, ambientLevel1);
  float i2 = calculateRelativeIntensity(processedReading2, ambientLevel2);
  float i3 = calculateRelativeIntensity(processedReading3, ambientLevel3);
  
  // Find the two strongest signals
  if (i1 >= i3 && i2 >= i3) {
    // Sensors 1 and 2 (left and right)
    float ratio = i1 / (i1 + i2);
    // Map ratio 0-1 to angle range -30 to 30 degrees
    return (ratio - 0.5) * 2 * sensorAngleLimit;
  } 
  else if (i1 >= i2 && i3 >= i2) {
    // Sensors 1 and 3 (right and middle)
    float ratio = i1 / (i1 + i3);
    // Map ratio 0-1 to angle range 0 to 30 degrees
    return ratio * sensorAngleLimit;
  }
  else {
    // Sensors 2 and 3 (left and middle)
    float ratio = i3 / (i2 + i3);
    // Map ratio 0-1 to angle range -30 to 0 degrees
    return (ratio - 1) * sensorAngleLimit;
  }
}

float FlameTriangulation::weightedAngularTriangulation() {
  // Calculate relative intensities
  float i1 = calculateRelativeIntensity(processedReading1, ambientLevel1);
  float i2 = calculateRelativeIntensity(processedReading2, ambientLevel2);
  float i3 = calculateRelativeIntensity(processedReading3, ambientLevel3);
  
  // Calculate weights
  float totalIntensity = i1 + i2 + i3;
  if (totalIntensity < 0.01) return 0.0; // Avoid division by zero
  
  // Weighted average based on sensor positions and intensities
  float weightedX = (sensor1X * i1 + sensor2X * i2 + sensor3X * i3) / totalIntensity;
  
  // Estimate angle based on weighted position
  // For small angles, tan(angle) ≈ angle in radians
  // Convert to degrees for output
  return atan2(weightedX, 10.0) * 180.0 / PI; // Assuming target is ~10cm away
}

float FlameTriangulation::getConfidence() {
  // Calculate relative intensities
  float i1 = calculateRelativeIntensity(processedReading1, ambientLevel1);
  float i2 = calculateRelativeIntensity(processedReading2, ambientLevel2);
  float i3 = calculateRelativeIntensity(processedReading3, ambientLevel3);
  
  // Total intensity as base confidence
  float totalIntensity = i1 + i2 + i3;
  float baseConfidence = constrain(totalIntensity / 1.5, 0.0, 1.0);
  
  // Adjust confidence based on consistency
  float consistency = 1.0;
  if (totalIntensity > 0.1) {
    // Check if intensity distribution makes sense for a point source
    // Intensity should decrease as we move away from the flame
    if ((i1 > i3 && i3 > i2) || (i2 > i3 && i3 > i1)) {
      consistency = 1.0; // Good pattern
    } else {
      consistency = 0.7; // Unexpected pattern
    }
  }
  
  return baseConfidence * consistency;
}

void FlameTriangulation::printDebugInfo() {
  Serial.println(F("------ Sensor Readings ------"));
  
  Serial.print(F("Raw: "));
  Serial.print(rawReading1);
  Serial.print(F(", "));
  Serial.print(rawReading2);
  Serial.print(F(", "));
  Serial.println(rawReading3);
  
  Serial.print(F("Processed: "));
  Serial.print(processedReading1);
  Serial.print(F(", "));
  Serial.print(processedReading2);
  Serial.print(F(", "));
  Serial.println(processedReading3);
  
  Serial.print(F("Relative Intensity: "));
  Serial.print(calculateRelativeIntensity(processedReading1, ambientLevel1), 2);
  Serial.print(F(", "));
  Serial.print(calculateRelativeIntensity(processedReading2, ambientLevel2), 2);
  Serial.print(F(", "));
  Serial.println(calculateRelativeIntensity(processedReading3, ambientLevel3), 2);
  
  Serial.print(F("Flame Detected: "));
  Serial.println(isFlameDetected() ? F("YES") : F("NO"));
  
  if (isFlameDetected()) {
    Serial.print(F("Flame Angle: "));
    Serial.print(getFlameAngle(), 1);
    Serial.println(F("°"));
    
    Serial.print(F("Confidence: "));
    Serial.print(getConfidence() * 100, 0);
    Serial.println(F("%"));
  }
  
  // Add ambient tracking debug info
  if (validSampleCount >= MIN_SAMPLES_FOR_DRIFT) {
    Serial.println(F("------ Ambient Tracking ------"));
    Serial.print(F("Current Avg: "));
    Serial.print(avgAmbient1, 1);
    Serial.print(F(", "));
    Serial.print(avgAmbient2, 1);
    Serial.print(F(", "));
    Serial.println(avgAmbient3, 1);
    
    Serial.print(F("Calibrated: "));
    Serial.print(ambientLevel1);
    Serial.print(F(", "));
    Serial.print(ambientLevel2);
    Serial.print(F(", "));
    Serial.println(ambientLevel3);
    
    Serial.print(F("Deviation: "));
    Serial.print(abs(avgAmbient1 - ambientLevel1), 1);
    Serial.print(F(", "));
    Serial.print(abs(avgAmbient2 - ambientLevel2), 1);
    Serial.print(F(", "));
    Serial.println(abs(avgAmbient3 - ambientLevel3), 1);
    
    Serial.print(F("Calibration Needed: "));
    Serial.println(calibrationNeeded ? F("YES") : F("NO"));
  }
  
  Serial.println();
}

// This method is not used in the current implementation,
// but keeping for future use if needed
float FlameTriangulation::angleFromIntensities(float intensity1, float intensity2, float intensity3) {
  // Implement if needed in the future
  return 0.0;
}

// This method is not used in the current implementation,
// but keeping for future use if needed
float FlameTriangulation::getConfidenceMetric() {
  return getConfidence();
} 