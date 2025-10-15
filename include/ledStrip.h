#ifndef LEDSTRIP_H
#define LEDSTRIP_H
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include "peakDetectorState.h"

#define LED_BUILTIN 2
#define LED_PIN 4
#define LED_COUNT 8

extern Adafruit_NeoPixel strip;
void activateLEDsOnce(uint8_t reset_pos);
void controlLEDs(PeakDetectorState *detector);

#endif