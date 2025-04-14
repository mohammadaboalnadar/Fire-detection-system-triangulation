# Fire Detection System Triangulation

An Arduino-based system for detecting and localizing flames using three flame sensors arranged in a linear configuration.

## Overview

This project implements a flame detection and triangulation system using three flame sensors. The system can:

1. Detect the presence of flames
2. Determine the angle to the flame source
3. Calculate a confidence level for the detection
4. Adapt to changing ambient light conditions via calibration
5. Display status and information on an LCD screen
6. Track flames with a servo motor
7. Control a water pump for fire suppression

## Hardware Requirements

- Arduino Uno/Nano/Mega
- 3 × Flame sensors (760-1100nm wavelength detection)
- 1 × Pushbutton (for calibration)
- 1 × LED (for status indication)
- 1 × 16x2 LCD display with I2C interface
- 1 × Servo motor (for flame tracking)
- 1 × Buzzer (for alerts)
- Jumper wires
- Breadboard

## Wiring Diagram

Connect the components as follows:

- **Flame Sensors**:
  - Right sensor: Analog output to A2
  - Middle sensor: Analog output to A1
  - Left sensor: Analog output to A0
  - VCC to 5V
  - GND to GND

- **Calibration Button**:
  - One terminal to digital pin 2
  - Other terminal to GND
  - Enable internal pull-up resistor in code

- **Status LED**:
  - Anode to digital pin 13 (through a current-limiting resistor)
  - Cathode to GND

- **LCD Display (I2C)**:
  - SDA to Arduino SDA (A4 on Uno/Nano)
  - SCL to Arduino SCL (A5 on Uno/Nano)
  - VCC to 5V
  - GND to GND

- **Servo Motor**:
  - Signal wire to digital pin 9
  - VCC to 5V (use external power if necessary)
  - GND to GND

- **Buzzer**:
  - Positive pin to digital pin (check Buzzer.h for the exact pin)
  - Negative pin to GND

## Sensor Arrangement

The sensors should be arranged in a straight line with:
- Right sensor at position (5cm, 0cm)
- Middle sensor at position (0cm, 0cm)
- Left sensor at position (-5cm, 0cm)

All sensors should face forward in the same direction.

## Software Architecture

The system consists of several modules:

1. **FlameTriangulation**:
   - Sensor reading processing
   - Flame detection algorithms
   - Angle estimation
   - Confidence calculation

2. **LCD Display**:
   - Status messages
   - Flame angle display
   - Calibration feedback
   - Non-blocking scrolling text capability

3. **Buzzer**:
   - Alert sounds for flame detection
   - Calibration tones
   - Startup sequence

4. **Servo Control**:
   - Scanning motion when no flame detected
   - Tracking motion when flame is detected
   - Smooth movement via interpolation

## Usage

1. **Initial Setup**:
   - Upload the code to your Arduino
   - At startup, the system initializes the LCD and displays a welcome message
   - System automatically performs calibration
   - Ensure no flames are present during calibration

2. **Operation**:
   - The system continuously monitors the sensors
   - LCD displays current status ("Monitoring..." or "FIRE DETECTED!")
   - When a flame is detected:
     - Status LED lights up
     - Buzzer sounds an alert
     - LCD shows the angle to the flame
     - Servo turns to point at the flame
     - Water pump activates when servo is properly aligned with flame
     - Detailed information is output via Serial for debugging

3. **Recalibration**:
   - Press the calibration button to recalibrate
   - LCD will show "Calibrating..." during this process
   - Useful when ambient light conditions change

## LCD Display

The 16x2 LCD display shows:
- Current system status in the top row
- For flames: angle in degrees in the bottom row
- During calibration: "Calibrating..." message
- During startup: initialization message

The LCD module supports scrolling long text without blocking the main loop, allowing the system to continue monitoring sensors while displaying information.

## Debugging

The system prints detailed debug information every second, including:
- Raw sensor readings
- Processed readings
- Relative intensities
- Detection status
- Flame angle and confidence (when detected)

## Theory of Operation

### Flame Detection

The flame sensors return a value between 0-1023, with lower values indicating higher flame intensity. The system detects a flame when any sensor reading is significantly below its calibrated ambient level.

### Angle Estimation

The angle to the flame source is estimated using several methods:

1. **Weighted Angular Triangulation**: When all three sensors detect the flame, a weighted average of sensor positions is used, with weights proportional to the relative flame intensity at each sensor.

2. **Dual-Sensor Estimation**: When only two sensors detect the flame, the ratio of their intensities is used to interpolate the angle.

3. **Single-Sensor Estimation**: When only one sensor detects the flame, the angle is estimated based on the sensor's position and detection range.

### Confidence Metric

The confidence level is calculated based on:
- Total flame intensity across all sensors
- Consistency of intensity distribution pattern
- Whether the detected pattern matches expected profiles for point sources

## Limitations

- The system is designed to detect only one flame source at a time
- Detection range is limited to approximately 0.8m for small flames
- Angular accuracy depends on flame intensity and distance
- Ambient light affects sensor performance and may require recalibration

## Future Improvements

- Implement distance estimation capability
- Implement multi-flame detection
- Support non-linear sensor arrangements
- Add automatic recalibration for changing light conditions
- Add data logging capabilities
- Implement wireless communication for remote monitoring