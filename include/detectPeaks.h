#ifndef DETECTPEAKS_H
#define DETECTPEAKS_H
#include <Arduino.h>
#include "peakDetectorState.h"

//void peakTest();
void detectPeaks(PeakDetectorState *detector);
void plot(int val1, int val2, int val3);

#endif