#include <Arduino.h>
#include <ESP32Servo.h>

#define SERVO_PIN 14

Servo myServo;

void setupServo(){
  myServo.setPeriodHertz(50);        // 50 Hz for servos
  myServo.attach(SERVO_PIN, 500, 2400);     // Pin 18, min/max pulse width
}


void testServo(){
    static uint8_t servoCounter = 0;
    static int servoTimestamp = -1;
    if (servoCounter >= 0) {
      if (millis()-servoTimestamp > 50) {  // change servo position every 50 ms
        servoTimestamp = millis();
        myServo.write(90 + 90 * cos(2 * PI * servoCounter / 200.0));  // Move servo between 0 and 180 degrees and back
        servoCounter++;
        if (servoCounter > 200) {  // whole cycle takes 10 seconds
          servoCounter = -1;
        }
      }
}