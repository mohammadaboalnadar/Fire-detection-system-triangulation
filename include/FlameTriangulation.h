#ifndef FLAME_TRIANGULATION_H
#define FLAME_TRIANGULATION_H

#include <Arduino.h>

class FlameTriangulation {
private:
    // Sensor positions in cm (linear arrangement)
    const float sensor1X = 5.0;  // Right sensor
    const float sensor2X = -5.0; // Left sensor
    const float sensor3X = 0.0;  // Middle sensor
    const float sensorY = 0.0;   // All at same height
    
    // Sensor characteristics
    const float sensorAngleLimit = 30.0; // Half of 60-degree detection angle
    const int threshold = 5;            // Detection threshold (raw value difference)
    
    // Raw and processed sensor readings
    int rawReading1;
    int rawReading2;
    int rawReading3;
    int processedReading1;
    int processedReading2;
    int processedReading3;
    
    // Circular buffer for smoothing readings
    static const int bufferSize = 5;
    int readingBuffer1[bufferSize];
    int readingBuffer2[bufferSize];
    int readingBuffer3[bufferSize];
    int bufferIndex;
    
    // Ambient tracking variables
    float avgAmbient1;
    float avgAmbient2;
    float avgAmbient3;
    unsigned long lastAmbientUpdate;
    unsigned int validSampleCount;
    unsigned long cooldownEndTime;
    static const int MIN_SAMPLES_FOR_DRIFT = 50;
    static const int DRIFT_WARNING_THRESHOLD = 2;
    
    // Methods
    void updateBuffers(int r1, int r2, int r3);
    int getSmoothedReading(int buffer[]);
    float angleFromIntensities(float intensity1, float intensity2, float intensity3);
    float weightedAngularTriangulation();
    float dualSensorEstimation();
    float getConfidenceMetric();
    void updateAmbientTracking(bool flameDetected);

public:
    // Calibration values (to be set during calibration)
    // Made public for distance estimation
    int ambientLevel1;
    int ambientLevel2;
    int ambientLevel3;
    
    // Calibration monitoring state
    bool calibrationNeeded;
    bool calibrationWarningTriggered;
    
    FlameTriangulation();
    
    // Main interface methods
    void calibrate(int reading1, int reading2, int reading3);
    void updateReadings(int reading1, int reading2, int reading3);
    
    // Flame detection results
    bool isFlameDetected();
    float getFlameAngle();
    float getConfidence();
    
    // Made public for distance estimation
    float calculateRelativeIntensity(int reading, int ambient);
    
    // Calibration monitoring
    void updateCalibrationMonitoring();
    float getCurrentAmbient1() { return avgAmbient1; }
    float getCurrentAmbient2() { return avgAmbient2; }
    float getCurrentAmbient3() { return avgAmbient3; }
    void resetCalibrationWarning();
    
    // Debug info
    void printDebugInfo();
};

#endif // FLAME_TRIANGULATION_H 