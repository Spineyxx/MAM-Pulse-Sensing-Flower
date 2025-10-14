#ifndef PEAKDETECTORSTATE_H
#define PEAKDETECTORSTATE_H
#include <Arduino.h>

typedef struct {
    int signalBuffer[50];
    int bufferIndex;
    int lastPeak;
    int thePeakBefore;
    char signalState;
    int detectionState;
} PeakDetectorState;

#endif
