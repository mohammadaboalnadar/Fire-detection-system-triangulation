# Fire Detection System Triangulation

An Arduino-based system for detecting and localizing flames using three flame sensors arranged in a linear configuration.

## Overview

This project implements a flame detection and triangulation system using three flame sensors. The system can:

1. Detect the presence of flames
2. Determine the angle to the flame source
3. Calculate a confidence level for the detection
4. Adapt to changing ambient light conditions via calibration
5. Display status, information, and environmental data (temperature/humidity) on an LCD screen
6. Track flames with a servo motor
7. Control a water pump for fire suppression
8. Provide visual alerts using siren-style LEDs
9. Monitor ambient light drift and suggest recalibration

## Hardware Requirements

- Arduino Uno/Nano/Mega
- 3 × Flame sensors (760-1100nm wavelength detection)
- 1 × Pushbutton (for calibration)
- 1 × LED (for status indication)
- 1 × 16x2 LCD display with I2C interface
- 1 × Servo motor (for flame tracking)
- 1 × Buzzer (for alerts)
- 1 x DHT11 Temperature and Humidity Sensor
- 2 x LEDs (for siren effect)
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
  - Positive pin to digital pin 7 (defined in `Buzzer.h`)
  - Negative pin to GND

- **DHT11 Sensor**:
  - Data pin to digital pin 3 (defined in `LCD.h`)
  - VCC to 5V
  - GND to GND

- **Siren LEDs**:
  - LED 1 Anode to digital pin 4 (defined in `main.cpp`)
  - LED 2 Anode to digital pin 5 (defined in `main.cpp`)
  - Both Cathodes to GND (through current-limiting resistors)

## Sensor Arrangement

The sensors should be arranged in a straight line with:
- Right sensor at position (5cm, 0cm)
- Middle sensor at position (0cm, 0cm)
- Left sensor at position (-5cm, 0cm)

All sensors should face forward in the same direction.

## Software Architecture

The system consists of several modules:

1. **FlameTriangulation**:
   - Sensor reading processing (smoothing, ambient tracking)
   - Flame detection algorithms
   - Angle estimation (weighted, dual-sensor, single-sensor)
   - Confidence calculation
   - Ambient drift detection logic

2. **LCD / LCDManager**:
   - Manages the 16x2 I2C LCD display
   - Displays status messages ("Monitoring...", "FIRE DETECTED!")
   - Shows flame angle when detected
   - Displays calibration feedback and warnings
   - Shows alternating temperature and humidity readings (from DHT sensor) when no flame is detected
   - Uses a buffering system for efficient updates (`LCD.cpp`)
   - `LCDManager` controls the refresh rate and decides what to display based on system state (flame, calibration needed, etc.)

3. **Buzzer**:
   - Controls the piezo buzzer
   - Provides audible alerts for flame detection (siren sound)
   - Plays tones for calibration start/finish and warnings
   - Plays a startup sequence sound

4. **Servo Control**:
   - Manages the servo motor connected to pin 9
   - Performs a scanning motion when no flame is detected
   - Tracks the detected flame angle using linear interpolation (lerp) for smooth movement

5. **Pump Control**:
   - Controls the water pump via a relay connected to pin 6
   - Activates the pump in pulses (duration/delay configurable) only when a flame is detected *and* the servo is aimed correctly (within a defined threshold)

6. **AmbientMonitor**:
   - Periodically checks for significant drift between the calibrated ambient light levels and the current running average
   - Triggers a calibration warning (visual on LCD, audible via Buzzer) if drift exceeds a threshold

7. **SirenLEDController**:
   - Controls two LEDs connected to pins 4 and 5
   - Creates an alternating flashing pattern (siren effect) when a flame is detected

8. **main.cpp**:
   - Initializes all hardware and software modules
   - Contains the main loop (`loop()`) that reads sensors, updates all subsystems, handles the calibration button press, and manages debug output

## Usage

1. **Initial Setup**:
   - Upload the code to your Arduino
   - At startup, the system initializes the LCD and displays a welcome message
   - System automatically performs initial calibration
   - Ensure no flames are present during calibration

2. **Operation**:
   - The system continuously monitors the flame sensors and the DHT sensor
   - LCD displays current status ("Monitoring..." with alternating Temp/Humidity, or "FIRE DETECTED!")
   - If ambient light drift is detected, a "RECALIBRATION RECOMMENDED!" message will appear periodically on the LCD, accompanied by a double beep.
   - When a flame is detected:
     - Status LED (pin 13) lights up (Note: This might be overridden by other LED logic if pin 13 is used elsewhere)
     - Siren LEDs (pins 4, 5) start flashing alternately
     - Buzzer sounds a siren alert
     - LCD shows "FIRE DETECTED!" and the estimated angle to the flame
     - Servo turns to point at the flame
     - Water pump activates in pulses when the servo is properly aligned with the flame
     - Detailed information is output via Serial for debugging

3. **Recalibration**:
   - Press and hold the calibration button (pin 2)
   - LCD will show "Calibrating..." and the buzzer will play a tone
   - Release the button. The system takes new ambient readings.
   - The buzzer plays a confirmation tone when finished.
   - Useful when ambient light conditions change significantly.

## LCD Display

The 16x2 LCD display shows:
- Current system status in the top row ("Monitoring...", "FIRE DETECTED!", "Calibrating...")
- For flames: angle in degrees in the bottom row ("Angle: X.Y deg")
- When monitoring: alternating temperature ("Temp: XX.X C") and humidity ("Humidity: XX.X%") in the bottom row
- During calibration: "Calibrating..." message
- When calibration is recommended: cycles between normal display, "RECALIBRATION RECOMMENDED!", and a comparison of saved vs. current ambient values.
- During startup: initialization message

The LCD module uses an internal buffer and update rate limiting (`LCD.cpp`, `LCDManager.cpp`) to avoid flickering and unnecessary writes. It also supports non-blocking scrolling text, though this feature is not currently used in the main application flow but is available in `LCD.cpp` and demonstrated in `LCD_example.ino`.

## Debugging

The system prints detailed debug information every second, including:
- Raw sensor readings
- Processed (smoothed) readings
- Relative intensities for each sensor
- Detection status (YES/NO)
- Flame angle and confidence (when detected)
- Ambient tracking info (current average vs. calibrated, deviation, calibration needed status)
- Pump status (ON/OFF)

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
- Whether the detected pattern matches expected profiles for point sources (e.g., intensity should generally decrease away from the center)

## Limitations

- The system is designed to detect only one flame source at a time
- Detection range is limited to approximately 0.8m for small flames
- Angular accuracy depends on flame intensity and distance
- Ambient light changes significantly affect sensor performance and require recalibration. The `AmbientMonitor` helps detect this.

## Future Improvements

- Implement distance estimation capability
- Implement multi-flame detection
- Support non-linear sensor arrangements
- Add automatic recalibration for changing light conditions
- Add data logging capabilities (e.g., to SD card)
- Implement wireless communication (e.g., ESP8266, LoRa) for remote monitoring/control
- Refine the pump control logic (e.g., variable pulse based on confidence/distance)