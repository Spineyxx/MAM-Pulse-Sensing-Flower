#ifndef PEAKDETECTORSTATE_H
#define PEAKDETECTORSTATE_H
#include <Arduino.h>

typedef struct {
    uint16_t signalBuffer[50]; // this holds the last 50 raw sensor readings
    uint8_t bufferIndex; // index for the signalBuffer array
    uint32_t lastPeak; // timestamp of the last detected peak
    uint32_t peakBeforeLast; // timestamp of the peak before the last detected peak
    char signalState; // derivation of signal -> 'r' for rising, 'f' for falling
    uint16_t hrInterval[5]; // holds the last 5 heart rate intervals
    uint8_t hrIntervalIndex; // index for the hrInterval array
    uint8_t detectionState; // state of the peak detection algorithm - 10 means fully initialized
    uint8_t peakDetected;
} PeakDetectorState;

#endif