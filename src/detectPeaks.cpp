#include "detectPeaks.h"

#include <Arduino.h>

#include "peakDetectorState.h"

void peakTest() { Serial.println("Peak detected!"); }

uint8_t detectPeaks(PeakDetectorState *detector) {
    static uint32_t potentialPeak = 0;
    // static unsigned long hrInterval = 0;
    static char derivPrev = 'r';  // r = rising; f = falling
    static uint16_t irSmooth_prev = 0;
    static uint16_t trigger = 0;
    static bool triggered = false;
    static int16_t triggerOld = 0;
    static bool triggeredOld = false;
    static uint32_t thePeakBefore = 0;
    static uint16_t hrInterval[5] = {0};
    static uint8_t hrIntervalIndex = 0;
    uint8_t peakDetected = 0;

    // Average of last 5 samples (irSmooth) and last 50 samples (irBaseline):
    uint32_t sumSmooth = 0;
    uint32_t sumBaseline = 0;

    for (uint8_t i = 0; i < 5;
         i++) {  // take last 5 of the array that holds last 50
        uint8_t index = ((detector->bufferIndex - i + 50) %
                         50);  // to not go beyond bounds of array
        sumSmooth += detector->signalBuffer[index];
    }
    uint32_t irSmooth = sumSmooth / 5;  //=average of last 5 values

    for (uint8_t i = 0; i < 50; i++) {
        sumBaseline += detector->signalBuffer[i];
    }
    uint32_t irBaseline = sumBaseline / 50;  //=average of last 50 values
    // note: calculation errors until the array is filled with respective amount
    // of values

    if (irSmooth > 40000) {  // FINGER ON = TRUE --> start Peak Detection
        if (detector->detectionState == 0) {
            detector->detectionState++;
        }

        // CURRENT PEAK DETECTION
        if (irSmooth >= irSmooth_prev) {
            // if current value is higher than the previous
            detector->signalState = 'r';  // signal is rising
        } else {
            detector->signalState = 'f';  // else it is falling
        }

        if (derivPrev == 'r' && detector->signalState == 'f') {
            // if it used to rise, but now falls
            potentialPeak = millis();  // get timestamp of potential peak

            switch (detector->detectionState) {
                case 1:  // finger just placed onto the sensor, first 4 peaks
                         // are accepted
                case 2:
                case 3:
                case 4:
                    thePeakBefore = detector->lastPeak;
                    detector->lastPeak = potentialPeak;
                    detector->detectionState++;
                    break;
                case 5:  // average interval of next 5 peaks is calculated
                case 6:
                case 7:
                case 8:
                case 9:
                    thePeakBefore = detector->lastPeak;
                    detector->lastPeak = potentialPeak;
                    hrIntervalIndex = detector->detectionState - 5;
                    hrInterval[hrIntervalIndex] =
                        detector->lastPeak - thePeakBefore;
                    detector->detectionState++;
                    break;
                case 10:  // ONLY AT 10 PEAK DETECTION IS VALID
                    uint32_t diff = potentialPeak - detector->lastPeak;
                    uint32_t hrIntervalSum = 0;
                    for (uint8_t i = 0; i <= 4;
                         i++) {  // build sum of last 5 intervals
                        hrIntervalSum += hrInterval[i];
                    }
                    if (diff > ((hrIntervalSum / 5) * 0.8)) {  // peak accepted
                        thePeakBefore = detector->lastPeak;
                        detector->lastPeak = potentialPeak;
                        if (hrIntervalIndex < 4) {
                            hrIntervalIndex++;
                        } else {
                            hrIntervalIndex = 0;
                        }
                        hrInterval[hrIntervalIndex] = diff;
                        peakDetected = 1;
                        // peakTest();

                        if (triggered) {
                            trigger = irBaseline - 200;
                            triggered = false;
                        } else {
                            trigger = irBaseline + 200;
                            triggered = true;
                        }
                    }
            }

            if (triggeredOld) {
                triggerOld = irBaseline - 200;
                triggeredOld = false;
            } else {
                triggerOld = irBaseline + 200;
                triggeredOld = true;
            }
        }
    } else {
        detector->detectionState = 0;
    }

    // plot(detector->signalBuffer[detector->bufferIndex], irSmooth, trigger,
    // (int)detector->detectionState);
    //plot(detector->signalBuffer[detector->bufferIndex], irSmooth, trigger);

    // save signal values for next iteration
    irSmooth_prev = irSmooth;
    derivPrev = detector->signalState;

    // move index to next position
    detector->bufferIndex = (detector->bufferIndex + 1) % 50;
    return peakDetected;
}

// Serial Plotter - HR Peak Detection
void plot(int amplitude, int irSmooth, int value) {
    Serial.print(amplitude);
    Serial.print("\t");
    Serial.print(irSmooth);
    Serial.print("\t");
    // Serial.print(trigger);
    // Serial.print("\t");
    Serial.print(value);
    Serial.print("\n");
}