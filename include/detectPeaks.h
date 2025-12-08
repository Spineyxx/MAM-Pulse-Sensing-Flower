#ifndef DETECTPEAKS_H
#define DETECTPEAKS_H
#include <Arduino.h>
#include "peakDetectorState.h"

// ADAPT THE THRESHOLD FOR FINGER PLACEMENT DETECTION HERE
// In different situations, the amount of received IR light varies greatly
// -> higher threshold leads to higher sensitivity regarding finger placement
// -> too high threshold might lead to no detection at all
#define FINGER_ON_THRESHOLD 40000

// How long to hold the 'calm' state once detected (milliseconds)
#define CALM_HOLD_MS 10000

void detectPeaks(PeakDetectorState *detector);
void decideCalmness(PeakDetectorState *detector);
void plot(uint16_t val1, u_int16_t val2, u_int16_t val3);
void printoutCalmnessDebug(PeakDetectorState *detector);
void getCurrentHRInterval(PeakDetectorState *detector);
void getCurrentHRIntervalAverage(PeakDetectorState *detector);

extern int currentHR;
extern int currentHRAverage;


#endif