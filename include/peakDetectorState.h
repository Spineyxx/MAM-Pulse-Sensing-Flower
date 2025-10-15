#ifndef PEAKDETECTORSTATE_H
#define PEAKDETECTORSTATE_H
#include <Arduino.h>

typedef struct {
    uint16_t signalBuffer[50];
    uint8_t bufferIndex;
    uint32_t lastPeak;
    char signalState;
    uint8_t detectionState;
} PeakDetectorState;

#endif
