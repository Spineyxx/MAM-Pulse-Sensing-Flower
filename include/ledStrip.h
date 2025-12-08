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

// für LED strip
#define COLORS_FADE_STEPS 255
#define DIMFACTOR 0.75
#define MAX_SWEEPS 4  // maximum simultaneous pulses
#define COLOR(r, g, b) ((uint32_t)(r) << 16 | (uint32_t)(g) << 8 | (uint32_t)(b))
#define CYAN   COLOR(0, 255, 255)
#define YELLOW COLOR(255, 255, 0) 
#define RED    COLOR(255, 0, 0)
#define PURPLE COLOR(255, 0, 255)
///

//für RING
#define RING_COLOR 0xFF00FF  // purple for the pulse
#define RING_DURATION 100    // ms, how long the ring stays lit
//

extern Adafruit_NeoPixel strip;

void activateLEDsOnce(uint8_t reset_pos);
void controlLEDs(PeakDetectorState *detector);
void testStripSetup();
void testStripLoop(PeakDetectorState *detector);
void functiontestLEDSTrip();

//für pulse-Funktionen

void setupStripPulse();
void strip_pulse(PeakDetectorState *detector);
void strip_pulseMulti(PeakDetectorState *detector);
void dimLeds(float dimFactor, uint8_t keepStandbyBrightness);
uint32_t fadeColor(uint32_t col1, uint32_t col2, int progress);
uint32_t getActColor();
uint32_t getHRColor(int currentHR);
extern MAX30105 particleSensor;
/////

//für RING
void ledRingControl();


void pulseRing();
void updateRing();
void loopRING(PeakDetectorState *detector);
//

#endif