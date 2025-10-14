#ifndef DETECTPEAKS_H
#define DETECTPEAKS_H
#include <Arduino.h>
#include "peakDetectorState.h"

void detectPeaks(PeakDetectorState* detector);
void plot(int amplitude, int irSmooth, int irBaseline, int trigger);


#endif