#ifndef LEDSTRIP_H
#define LEDSTRIP_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "peakDetectorState.h"
#include "MAX30105.h"           // library: Sparkfun MAX3010x

#define LED_BUILTIN 2
#define LED_PIN 12
#define LED_COUNT 23
#define LED_COUNT_STRIP 16
#define LED_COUNT_RING 7

// used in fadeColor
#define COLORS_FADE_STEPS 255

// used in dimLeds
#define DIMFACTOR 0.75

// used in stripPulseMulti
#define MAX_SWEEPS 4  // maximum simultaneous pulses

// used in getActColor
#define COLOR(r, g, b) ((uint32_t)(r) << 16 | (uint32_t)(g) << 8 | (uint32_t)(b))
#define CYAN   COLOR(0, 255, 255)
#define YELLOW COLOR(255, 255, 0) 
#define RED    COLOR(255, 0, 0)
#define PURPLE COLOR(255, 0, 255)

extern Adafruit_NeoPixel strip;
extern MAX30105 particleSensor;

// used for pulse functions
void setupStripPulse();
void dimLeds(float dimFactor, uint8_t keepStandbyBrightness);
uint32_t fadeColor(uint32_t col1, uint32_t col2, int progress);
uint32_t getActColor();
uint32_t getHRColor(int currentHR);
void stripPulse(PeakDetectorState *detector);
void stripPulseMulti(PeakDetectorState *detector);



// used for test-functions
// for strip
void functiontestLEDSTrip();
// for ring
void ledRingControl();



// used in old functions
#define RING_COLOR 0xFF00FF  // purple for the pulse
#define RING_DURATION 100    // ms, how long the ring stays lit
void activateLEDsOnce(uint8_t reset_pos);
void controlLEDs(PeakDetectorState *detector);
void testStripSetup();
void testStripLoop(PeakDetectorState *detector);
void pulseRing();
void updateRing();
void loopRING(PeakDetectorState *detector);


#endif