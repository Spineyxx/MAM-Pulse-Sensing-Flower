#include <Arduino.h>
#include <ESP32Servo.h>

#include "flowerState.h"
#include "peakDetectorState.h"

#define SERVO_PIN 14

Servo myServo;

void setupServo() {
    myServo.setPeriodHertz(50);            // 50 Hz for servos
    myServo.attach(SERVO_PIN, 500, 2500);  // Pin 18, min/max pulse width
    myServo.write(FLOWER_CLOSED_ANGLE);    // start with flower closed
}

// Note: myServo.write() is not executed directly but through setSafeAngle() to
// ensure angle is within the operational range (especially during testing) 
void setSafeAngle(uint8_t angle) {  // credits to the servo library, inspo was
                                    // taken from the write() function
    if (angle > FLOWER_OPEN_ANGLE) {
        angle = FLOWER_OPEN_ANGLE;
    }
    if (angle < FLOWER_CLOSED_ANGLE) {
        angle = FLOWER_CLOSED_ANGLE;
    }
    myServo.write(angle);
}

void closeFlower(FlowerState* flower) {
    if (flower->motion != 0) {  // if it would be closed (0) -> no need to do anything

        if (millis() - flower->lastMotionTimestamp > 50) {
            flower->lastMotionTimestamp = millis();
            flower->motion = 3;  //  closing
            
            flower->servoCounter--;
            flower->currentAngle = FLOWER_CLOSED_ANGLE + ((FLOWER_OPEN_ANGLE - FLOWER_CLOSED_ANGLE) * 0.5 * (1 - cos(PI * flower->servoCounter / 50.0)));
            setSafeAngle(flower->currentAngle);
        }     

        if (flower->servoCounter <= 0) {
            flower->servoCounter = 0;  // avoid overflow
            flower->motion = 0;  // flower is closed
        }
    }
}

void openFlower(FlowerState* flower) {
    if (flower->motion != 2) {  // if it would be open (2) -> no need to do anything

        if (millis() - flower->lastMotionTimestamp > 50) {
            flower->lastMotionTimestamp = millis();
            flower->motion = 1;  //  opening

            flower->servoCounter++;
            flower->currentAngle = FLOWER_CLOSED_ANGLE + ((FLOWER_OPEN_ANGLE - FLOWER_CLOSED_ANGLE) * 0.5 * (1 - cos(PI * flower->servoCounter / 50.0)));
            setSafeAngle(flower->currentAngle);
        } 
        
        if (flower->servoCounter >= 50) {
            flower->servoCounter = 50;  // avoid overflow
            flower->motion = 2;  // flower is closed
        }

    }
}


// Handles flower opening and closing motion
void handleFlower(FlowerState* flower, PeakDetectorState* detector, uint8_t peak) {
    static uint8_t peakCounter = 0;

    // for testing - replace with logic on when to open and close flower
    
    if (detector->detectionState != 10) {
        peakCounter = 0;  // reset counter when finger is off
        closeFlower(flower);
    }else{

        if (peak == 1) {
            peakCounter++;
        }
    
        if (peakCounter >= 20) {
            closeFlower(flower);
        } else if (peakCounter >= 5) {
            openFlower(flower);
        }
    }
        Serial.print(peakCounter);
        Serial.print("\t");
        Serial.print(flower->motion);
        Serial.print("\t");
        Serial.print(flower->servoCounter);
        Serial.print("\t");
        Serial.print(flower->currentAngle);
        Serial.print("\t");
        Serial.print(myServo.read());
        Serial.print("\n");
    }

// Brainstorming Methods
//  - have one method "toggleFlower()" (have to know and maybe check current
//  state)
//  - instead have one method "openFlower()" and one method "closeFlower()"
//  - flower struct with current angle and bool for if closed or open? ->

// have closeFlower and openFlower function
// closeFlower also when finger off, detector übergabe für handleFlower

// read current angle every time or assume that it was set correctly?