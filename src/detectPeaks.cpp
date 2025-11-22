#include "detectPeaks.h"
#include <Arduino.h>
#include "peakDetectorState.h"

// ADAPT THE THRESHOLD FOR FINGER PLACEMENT DETECTION HERE
// In different situations, the amount of received IR light varies greatly
// -> higher threshold leads to higher sensitivity regarding finger placement
// -> too high threshold might lead to no detection at all though
#define FINGER_ON_THRESHOLD 40000

void detectPeaks(PeakDetectorState *detector) {
    uint32_t potentialPeak = 0;
    static char derivPrev = 'r';  // r = rising; f = falling
    static uint16_t irSmooth_prev = 0;
    // Variables for Serial Plotter visualization:
    static uint16_t trigger = 0;
    static bool triggered = false;
    static int16_t triggerOld = 0;
    static bool triggeredOld = false;

    detector->peakDetected = 0; // reset peak detected flag

    // Calculating the average of last 5 samples (irSmooth) and last 50 samples (irBaseline):
    uint32_t sumSmooth = 0;
    uint32_t sumBaseline = 0;

    for (uint8_t i = 0; i < 5; i++) {  // take last 5 of the array that holds last 50
        uint8_t index = ((detector->bufferIndex - i + 50) % 50);  // to not go beyond bounds of array
        sumSmooth += detector->signalBuffer[index];
    }
    uint32_t irSmooth = sumSmooth / 5;  //=average of last 5 values

    for (uint8_t i = 0; i < 50; i++) {
        sumBaseline += detector->signalBuffer[i];
    }
    uint32_t irBaseline = sumBaseline / 50;  //=average of last 50 values
    
    // NOTE: The creators of this code are aware that there will be initial
    // errors in the baseline and smoothing calculation, until the arrays are filled
    // up. However, we do not see them as critical, since this startup phase is so
    // short that it does not affect peak detection in a relevant way.

    if (irSmooth > FINGER_ON_THRESHOLD) {  // FINGER ON = TRUE --> start Peak Detection
        if (detector->detectionState == 0) {
            detector->detectionState++; 
            //detectionState is always > 0 as soon as finger is placed on sensor
        }

        if (irSmooth >= irSmooth_prev) { // if current value is higher than the previous
            detector->signalState = 'r';  // signal is rising
        } else {
            detector->signalState = 'f';  // else it is falling
        }

        if (derivPrev == 'r' && detector->signalState == 'f') { // if it used to rise, but now falls
            potentialPeak = millis();  // get timestamp of potential peak

            switch (detector->detectionState) {
                case 1:
                case 2:
                case 3:
                case 4:
                    // first 4 peaks are accepted
                    detector->peakBeforeLast = detector->lastPeak;
                    detector->lastPeak = potentialPeak;
                    detector->detectionState++;
                    break;
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                    // next 5 peaks are also accepted
                    // + average interval of these 5 peaks is calculated and saved to hrInterval array
                    detector->peakBeforeLast = detector->lastPeak;
                    detector->lastPeak = potentialPeak;
                    detector->hrIntervalIndex = detector->detectionState - 5;
                    detector->hrInterval[detector->hrIntervalIndex] =
                        detector->lastPeak - detector->peakBeforeLast;
                    detector->detectionState++;
                    break;
                case 10:  // ONLY AT DETECTOR STATE 10 -> PEAK DETECTION IS VALID
                    uint32_t diff = potentialPeak - detector->lastPeak;
                    uint32_t hrIntervalSum = 0;
                    for (uint8_t i = 0; i <= 4; i++) {  // build sum of last 5 intervals
                        hrIntervalSum += detector->hrInterval[i];
                    }
                    if (diff > ((hrIntervalSum / 5) * 0.8)) {
                        //the peak is valid when the interval is > 80% of the average of the last 5 intervals
                        detector->peakBeforeLast = detector->lastPeak;
                        detector->lastPeak = potentialPeak;
                        if (detector->hrIntervalIndex < 4) { // move index forward
                            detector->hrIntervalIndex++;
                        } else {
                            detector->hrIntervalIndex = 0;
                        }
                        detector->hrInterval[detector->hrIntervalIndex] = diff; //save the new interval
                        detector->peakDetected = 1; // set return variable to 1 to indicate peak detection
                        // peakTest();

                        if (triggered) { //this is just for visualization in the serial plotter
                            trigger = irBaseline - 200;
                            triggered = false;
                        } else {
                            trigger = irBaseline + 200;
                            triggered = true;
                        }
                    }
            }

            if (triggeredOld) { //for visualisation of the previous peak detection in the serial plotter
                triggerOld = irBaseline - 200;
                triggeredOld = false;
            } else {
                triggerOld = irBaseline + 200;
                triggeredOld = true;
            }
        }
    } else {
        detector->detectionState = 0; // FINGER ON = FALSE --> reset detection state to 0
    }

    // Uncomment the line below to visualize signal, smoothed signal and trigger in Serial Plotter
    //plot(detector->signalBuffer[detector->bufferIndex], irSmooth, trigger);

    // save signal values for next iteration
    irSmooth_prev = irSmooth;
    derivPrev = detector->signalState;

    // move index to next position
    detector->bufferIndex = (detector->bufferIndex + 1) % 50;
}

// Serial Plotter - HR Peak Detection - plot 3 signals
void plot(int val1, int val2, int val3) {
    Serial.print(val1);
    Serial.print("\t");
    Serial.print(val2);
    Serial.print("\t");
    Serial.print(val3);
    Serial.print("\n");
}