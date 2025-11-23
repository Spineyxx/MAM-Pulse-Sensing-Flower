#ifndef PEAKDETECTORSTATE_H
#define PEAKDETECTORSTATE_H
#include <Arduino.h>

typedef struct {
    uint16_t signalBuffer[50]; // this holds the last 50 raw sensor readings (not averaged)
    uint8_t bufferIndex; // index for the signalBuffer array
    uint32_t lastPeak; // timestamp of the last detected peak
    uint32_t peakBeforeLast; // timestamp of the peak before the last detected peak
    char signalState; // derivation of signal -> 'r' for rising, 'f' for falling
    uint16_t hrInterval[20]; // holds the last 20 heart rate intervals
    uint8_t hrIntervalIndex; // index for the hrInterval array
    uint8_t detectionState; // state of the peak detection algorithm - 10 means fully initialized
    uint32_t fingerOnSinceMillis; // timestamp when finger was first detected on sensor
    uint8_t peakDetected; //is 1 if peak detected in current cycle, else 0
    char chillVariable; //Variablenname von meiner Mitbewohnerin ;) -> 'c' for calm, 'u' for undefined
    float hrRatio; // ratio of last 10 HR intervals to the 10 before that
    uint32_t calmUntilMillis; // timestamp until which 'c' should be held
} PeakDetectorState;

#endif