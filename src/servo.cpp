#include <Arduino.h>
#include <ESP32Servo.h>
#include "flowerState.h"

#define SERVO_PIN 14

Servo myServo;

void setupServo(){
  myServo.setPeriodHertz(50);        // 50 Hz for servos
  myServo.attach(SERVO_PIN, 500, 2500);     // Pin 18, min/max pulse width
}

//Note: myServo.write() is not executed directly but through setSafeAngle() to ensure angle is within the operational range
void setSafeAngle(uint8_t angle){ //credits to the servo library, inspo was taken from the write() function
  if(angle < FLOWER_OPEN_ANGLE){
    angle = FLOWER_OPEN_ANGLE;
  }
  if(angle > FLOWER_CLOSED_ANGLE){
    angle = FLOWER_CLOSED_ANGLE;
  }
  myServo.write(angle); 
  
}

//Handles flower opening and closing motion
void handleFlower(FlowerState *flower){
    static uint8_t servoCounter = 0;
    static int32_t servoTimestamp = 0;

    if (servoCounter >= 0) {

      if (millis()-servoTimestamp > 50) {  // change servo position every 50 ms
        servoTimestamp = millis();
        setSafeAngle(FLOWER_OPEN_ANGLE + ((FLOWER_CLOSED_ANGLE - FLOWER_OPEN_ANGLE) * 0.5 * (1 + cos(2 * PI * servoCounter / 200.0))));
        servoCounter++; //0 bis 200
        if (servoCounter > 200) {  // whole cycle takes 10 seconds
          servoCounter = 0;
        }

    }
  }
}

//Brainstorming Methods
// - have one method "toggleFlower()" (have to know and maybe check current state)
// - instead have one method "openFlower()" and one method "closeFlower()"
// - flower struct with current angle and bool for if closed or open? -> 
