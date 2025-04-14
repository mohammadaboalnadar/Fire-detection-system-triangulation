#include <Arduino.h>
/*
  AnalogReadSerial

  Reads analog inputs on pins 0, 1, and 2, prints the results to the Serial Monitor.
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).

  This example code is in the public domain.
*/

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the inputs on analog pins 0, 1, and 2:
  int sensorValue1 = analogRead(A0);
  int sensorValue2 = analogRead(A1);
  int sensorValue3 = analogRead(A2);
  
  // print out all values on the same line separated by spaces:
  Serial.print(sensorValue1);
  Serial.print(" ");
  Serial.print(sensorValue2);
  Serial.print(" ");
  Serial.println(sensorValue3);
  
  delay(1);  // delay in between reads for stability
}
