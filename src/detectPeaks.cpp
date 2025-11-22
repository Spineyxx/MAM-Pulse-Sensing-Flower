#include "detectPeaks.h"
#include <Arduino.h>
#include "peakDetectorState.h"

// ADAPT THE THRESHOLD FOR FINGER PLACEMENT DETECTION HERE
// In different situations, the amount of received IR light varies greatly
// -> higher threshold leads to higher sensitivity regarding finger placement
// -> too high threshold might lead to no detection at all though
#define FINGER_ON_THRESHOLD 40000

// How long to hold the 'calm' state once detected (milliseconds)
#define CALM_HOLD_MS 15000

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

    uint32_t sumSmooth = 0;
    uint32_t sumBaseline = 0;

    // Calculating the average of last 5 samples (irSmooth) and last 50 samples (irBaseline):
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
                    detector->hrInterval[detector->hrIntervalIndex] = detector->lastPeak - detector->peakBeforeLast;
                    detector->detectionState++;
                    break;
                case 10:  // ONLY AT DETECTOR STATE 10 -> PEAK DETECTION IS VALID
                    uint32_t diff = potentialPeak - detector->lastPeak;
                    uint32_t hrIntervalFiveSum = 0;
                    // calculate sum of last 5 intervals
                    for (uint8_t i = 0; i < 5; i++) {
                        int idx = detector->hrIntervalIndex - i;
                        if (idx < 0) idx += 20; // avoid negative index after 21st interval
                        hrIntervalFiveSum += detector->hrInterval[idx];
                    }
                    if (diff > ((hrIntervalFiveSum / 5) * 0.8)) { //the peak is valid when the interval is > 80% of the average of the last 5 intervals
                        detector->peakBeforeLast = detector->lastPeak;
                        detector->lastPeak = potentialPeak;
                        if (detector->hrIntervalIndex < 19) { // move index forward
                            detector->hrIntervalIndex++;
                        } else {
                            detector->hrIntervalIndex = 0;
                        }
                        detector->hrInterval[detector->hrIntervalIndex] = diff; //save the new interval
                        detector->peakDetected = 1; // set return variable to 1 to indicate peak detection

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

    decideCalmness(detector); // decide calmness state based on HR intervals

    // save signal values for next iteration
    irSmooth_prev = irSmooth;
    derivPrev = detector->signalState;

    // move index to next position
    detector->bufferIndex = (detector->bufferIndex + 1) % 50;

}

void decideCalmness(PeakDetectorState *detector) {
    if (detector->detectionState == 10) { //only check for calmness if finger is on and peak detection is initialized

        uint32_t hrIntervalSum = 0;
        uint8_t intervalsFound = 0;

        // Determine index of the most-recent stored interval (previous slot)
        uint8_t prevIndex = detector->hrIntervalIndex - 1;
        if (prevIndex < 0) prevIndex += 20;

        // Get last 20 intervals (or fewer if not yet available)
        for (uint8_t i = 0; i < 20 && intervalsFound < 5; i++) {
            int j = prevIndex - i;
            if (j < 0) j += 20;
            uint16_t val = detector->hrInterval[j];
            if (val != 0) {
                hrIntervalSum += val;
                intervalsFound++;
            }
        }

        if (intervalsFound == 0) { //cannot happen due to detectionState but still just in case
            detector->chillVariable = 'u';
        } else {
            uint32_t hrIntervalAvg = hrIntervalSum / intervalsFound;

            // Use the most-recent stored interval for the deviation check
            uint16_t lastInterval = detector->hrInterval[prevIndex];
            // Serial.println(lastInterval/hrIntervalAvg*100); //debug output to see deviation in percent

            // upper bound in case of missed peak
            if ((uint32_t)lastInterval >= hrIntervalAvg * 85 / 100 && (uint32_t)lastInterval <= hrIntervalAvg * 115 / 100) {
                detector->chillVariable = 'c';
                detector->calmUntilMillis = millis() + CALM_HOLD_MS;
            } else if ((uint32_t)(detector->calmUntilMillis - (uint32_t)millis()) > 0) {
                // still within hold window
                detector->chillVariable = 'c';
            } else {
                detector->chillVariable = 'u';
            }
        }
    }else{
        detector->chillVariable = 'u'; // undefined
    }
    plot(detector->detectionState, detector->peakDetected, detector->chillVariable);
}

// Serial Plotter function - HR Peak Detection - plot 3 signals
void plot(int val1, int val2, int val3) {
    Serial.print(val1);
    Serial.print("\t");
    Serial.print(val2);
    Serial.print("\t");
    Serial.print(val3);
    Serial.print("\n");
}