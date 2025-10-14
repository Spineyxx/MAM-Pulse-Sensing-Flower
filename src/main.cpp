#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "readSensor.h"
#include "detectPeaks.h"
#include "peakDetectorState.h"

PeakDetectorState detector = {
    .signalBuffer = {0},
    .bufferIndex = 0,
    .lastPeak = 0, //millis()
    .thePeakBefore = 0,
    .signalState = 'r', //r=rising, f=falling
    .detectionState = 0 //if(detectionState == 4)
};

// the setup function is called once (when the program starts)
void setup() {
  Serial.begin(115200);
  setupSensor();
}

// after the setup() function returned, the loop function is called in an endless loop 
void loop() {
  getPulseOxySignal(&detector);
  detectPeaks(&detector);

}

