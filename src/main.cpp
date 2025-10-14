#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "readSensor.h"
#include "detectPeaks.h"
#include "peakDetectorState.h"
#include "ledStrip.h"

PeakDetectorState detector = {
    .signalBuffer = {0},
    .bufferIndex = 0,
    .lastPeak = 0,
    .thePeakBefore = 0,
    .signalState = 'r',
    .detectionState = 0
};


// the setup function is called once (when the program starts)
void setup() {
  Serial.begin(115200);
  strip.begin();
  for (int i = 0; i< LED_COUNT; i++)  { 
    strip.setPixelColor(i,0,0,0); 
  }
  strip.show();       
  //setupSensor();                                                             // damit ohne Pulssensor getestet werden kann ~LINDA
}

// after the setup() function returned, the loop function is called in an endless loop 
void loop() {
  //getPulseOxySignal(&detector);                                              // damit ohne Pulssensor getestet werden kann ~LINDA 
  //detectPeaks(&detector);                                                    // damit ohne Pulssensor getestet werden kann ~LINDA
  detector.detectionState = 4; //                                              // damit ohne Pulssensor getestet werden kann ~LINDA
  static uint32_t ts = 0; //                                                   // damit ohne Pulssensor getestet werden kann ~LINDA
  if(millis() - ts > 1000){ //                                                 // damit ohne Pulssensor getestet werden kann ~LINDA
    ts = millis();
    activateLEDsOnce(1);  //resettet pos
  }
  controlLEDs(&detector); // Function for controlling the LED Strip
}