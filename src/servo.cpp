#include <Arduino.h>
#include <ESP32Servo.h>

#include "flowerState.h"
#include "peakDetectorState.h"

#define SERVO_PIN 14

Servo myServo;

void setupServo() {
    myServo.setPeriodHertz(50);            // 50 Hz for servos
    myServo.attach(SERVO_PIN, 500, 2500);  // Pin 18, min/max pulse width
}

// Note: myServo.write() is not executed directly but through setSafeAngle() to
// ensure angle is within the operational range -> set the limits in the header file
void setSafeAngle(uint8_t angle) {  // credits to the servo library, inspo was
                                    // taken from the write() function
    if (angle < FLOWER_OPEN_ANGLE) {
        angle = FLOWER_OPEN_ANGLE;
    }
    if (angle > FLOWER_CLOSED_ANGLE) {
        angle = FLOWER_CLOSED_ANGLE;
    }
    myServo.write(angle);
}

// Handles flower opening and closing motion
void handleFlower(FlowerState* flower, uint8_t peak) {
    static uint8_t peakCounter = 0;
    static uint8_t servoCounter = 0;
    static int32_t servoTimestamp = 0;

    if (peak == 1) {    // A peak was detected
        peakCounter++;  // note: DONT FORGET TO RESET AFTER FINGER REMOVAL
    }

    // for testing, flower opens after 10 peaks and closes after next 10 peaks

    if (peakCounter >= 10) {
        if (millis() - servoTimestamp > 50) {  // change servo position every 50 ms
            servoTimestamp = millis();

            if (peakCounter < 20) {// Opening motion (90° -> 0°)
              flower->motion = 1; // opening  
              setSafeAngle(FLOWER_CLOSED_ANGLE - ((FLOWER_CLOSED_ANGLE - FLOWER_OPEN_ANGLE) * 0.5 * (1 + cos(2 * PI * servoCounter / 200.0))));
            } else {// Closing motion (0° -> 90°)
              flower->motion = 3; // closing  
              setSafeAngle(FLOWER_OPEN_ANGLE + ((FLOWER_CLOSED_ANGLE - FLOWER_OPEN_ANGLE) * 0.5 * (1 + cos(2 * PI * servoCounter / 200.0))));
                if (servoCounter >= 200) {
                    peakCounter = 0;  // Reset counter after full close
                }
            }
            flower->currentAngle = myServo.read();

            if(flower->currentAngle == FLOWER_OPEN_ANGLE) {
                flower->motion = 2; // open
            } else if(flower->currentAngle == FLOWER_CLOSED_ANGLE) {
                flower->motion = 0; // closed
            }

            servoCounter++;
            if (servoCounter > 200) {
                servoCounter = 0;
            }
        }
    }
}

// Brainstorming Methods
//  - have one method "toggleFlower()" (have to know and maybe check current
//  state)
//  - instead have one method "openFlower()" and one method "closeFlower()"
//  - flower struct with current angle and bool for if closed or open? ->
