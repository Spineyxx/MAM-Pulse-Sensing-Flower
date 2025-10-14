#ifndef PEAKDETECTORSTATE_H
#define PEAKDETECTORSTATE_H
#include <Arduino.h>

typedef struct
{
    int signalBuffer[50];
    int bufferIndex;
    uint32_t lastPeak;
    char signalState;
    int detectionState;
} PeakDetectorState;

#endif
