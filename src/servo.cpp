#include <Arduino.h>
#include <ESP32Servo.h>

#include "servo.h"
#include "flowerState.h"
#include "peakDetectorState.h"

#define SERVO_PIN 14

Servo myServo;

void setupServo() {
    myServo.setPeriodHertz(50);            // 50 Hz for servos
    myServo.attach(SERVO_PIN, 500, 2500);  // Pin 18, min/max pulse width
    myServo.write(FLOWER_CLOSED_ANGLE);    // start with flower closed -> snaps shut after startup
}

// Note: myServo.write() is not executed directly but through setSafeAngle() to
// ensure angle is within the operational range (especially to prevent mechanical issues during testing) 
void setSafeAngle(uint8_t angle) {  // credits to the servo library, inspo taken from the write() function :)
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

        if (millis() - flower->lastMotionTimestamp > 20) {
            flower->lastMotionTimestamp = millis();
            flower->motion = 3;  //  closing
            
            flower->servoCounter--;
            flower->currentAngle = FLOWER_CLOSED_ANGLE + ((FLOWER_OPEN_ANGLE - FLOWER_CLOSED_ANGLE) * 0.5 * (1 - cos(PI * flower->servoCounter / 100.0)));
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

        if (millis() - flower->lastMotionTimestamp > 20) {
            flower->lastMotionTimestamp = millis();
            flower->motion = 1;  //  opening

            flower->servoCounter++;
            flower->currentAngle = FLOWER_CLOSED_ANGLE + ((FLOWER_OPEN_ANGLE - FLOWER_CLOSED_ANGLE) * 0.5 * (1 - cos(PI * flower->servoCounter / 100.0)));
            setSafeAngle(flower->currentAngle);
        } 
        
        if (flower->servoCounter >= 100) {
            flower->servoCounter = 100;  // avoid overflow
            flower->motion = 2;  // flower is closed
        }

    }
}


// Handles flower opening and closing algorithm based on peak detection
void handleFlower(FlowerState* flower, PeakDetectorState* detector) {
    static uint16_t peakCounter = 0; //get rid of peakCounter later

    //*********************** TESTING BLOCK ***********************
    // replace with logic on when to open and close flower
    
    if (detector->detectionState != 10) {
        peakCounter = 0;  // reset counter when finger is off
        closeFlower(flower);
    }else{
        if (detector->chillVariable == 'c') {
            openFlower(flower);
        }
    //    if (detector->peakDetected == 1) {
    //        peakCounter++; //count detected peaks
    //    }
    
    //    if (peakCounter >= 15) { //close after 15 peaks detected
    //        closeFlower(flower);
    //    } else if (peakCounter >= 2) { //open after 2 peaks detected
    //        openFlower(flower);
    //    }
    }
}
    //printoutFlowerDebug(flower, peakCounter); // debug output to serial monitor

    //*************************************************************


    void printoutFlowerDebug(FlowerState* flower, uint16_t peakCounter) {
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