#ifndef DETECTPEAKS_H
#define DETECTPEAKS_H
#include <Arduino.h>
#include "peakDetectorState.h"

void detectPeaks(PeakDetectorState *detector);
void decideCalmness(PeakDetectorState *detector);
void plot(uint16_t val1, u_int16_t val2, u_int16_t val3);
void printoutCalmnessDebug(PeakDetectorState *detector);

#endif