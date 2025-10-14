#ifndef READSENSOR_H
#define READSENSOR_H
#include <Arduino.h>
#include "peakDetectorState.h"

void setupSensor();
void getPulseOxySignal(PeakDetectorState* detector);

#endif