#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include "detectPeaks.h"
#include "leds.h"
#include "peakDetectorState.h"
#include "flowerState.h"
#include "readSensor.h"
#include "servo.h"
#include "btSound.h"

PeakDetectorState detector = {.signalBuffer = {0},
                              .bufferIndex = 0,
                              .lastPeak = 0,
                              .peakBeforeLast = 0,
                              .signalState = 'r',
                              .hrInterval = {0},
                              .hrIntervalIndex = 0,
                              .detectionState = 0,
                              .fingerOnSinceMillis = 0,
                              .peakDetected = 0,
                              .chillVariable = 'u',
                              .hrRatio = 0.0,
                              .calmUntilMillis = 0,
                              .currentHR = 0,
                              .currentHRAverage = 0};

FlowerState flower = {.currentAngle = FLOWER_CLOSED_ANGLE,
                    .servoCounter = 0,
                    .motion = 0, //0 = closed, 1 = opening, 2 = open, 3 = closing
                    .lastMotionTimestamp = 0}; //0 = closed, 1 = opening, 2 = open, 3 = closing
                            
// the setup function is called once (when the program starts)
void setup() {
    Serial.begin(115200);
    setupServo();
    setupSensor();
    setupStripPulse();
    setupBT();
}

// after the setup() function returned, the loop function is called in an endless loop
void loop() {
    getPulseOxySignal(&detector);
    detectPeaks(&detector); 
    //getCurrentHRInterval(&detector); // this gets the current HR in bpm
    getCurrentHRIntervalAverage(&detector); // this gets the curretn average HR in bpm
    //setSafeAngle(45); // use this to set servo to open position if you insert the servo into the flower
    handleFlower(&flower, &detector);
    handleBT(&flower, &detector);
    //stripPulse(&detector); // goes through fixed color sequence
    stripPulseMulti(&detector); // color sequence dependent on HR
    //ledRingControl(); // use this to light up/test LED ring
    //functiontestLEDSTrip(); // use this to test if LED strip works
    
}

// NOTES ON DATA TYPES:
// uint8_t für Werte von 0-255
// uint16_t für Werte von 0-65535
// uint32_t für Werte von 0-4.294.967.295
// int16_t für Werte von -32.768 bis 32.767
// int32_t für Werte von -2.147.483.648 bis 2.
