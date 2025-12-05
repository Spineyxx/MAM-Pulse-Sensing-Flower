#include "detectPeaks.h"
#include <Arduino.h>
#include "peakDetectorState.h"

void detectPeaks(PeakDetectorState *detector) {
    static char derivPrev = 'r';  // r = rising; f = falling
    static uint16_t irSmooth_prev = 0;
    uint32_t potentialPeak = 0;
    uint32_t sumSmooth = 0;
    uint32_t sumBaseline = 0; //for now just to visualize trigger in serial plotter

    // Variables for Serial Plotter visualization:
    static uint16_t trigger = 0;
    static bool triggered = false;

    detector->peakDetected = 0; // reset peak detected flag

    // Calculating the average of last 5 samples (irSmooth) and last 50 samples (irBaseline):
    for (uint8_t i = 0; i < 5; i++) {  // take last 5 of the array that holds last 50
        int idx = (int)detector->bufferIndex - (int)i; // apparently this needs to be int to avoid underflow according to GPT
        if (idx < 0) idx += 50;
        uint8_t index = (uint8_t)idx;
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
    // short that it does not affect peak detection in a relevant way :)

    if (irSmooth > FINGER_ON_THRESHOLD) {  // FINGER ON = TRUE --> start Peak Detection
        if (detector->detectionState == 0) {
            detector->detectionState++; 
            detector->fingerOnSinceMillis = millis();
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
        }
    } else {
        detector->detectionState = 0; // FINGER ON = FALSE --> reset detection state to 0
        detector->fingerOnSinceMillis = 0;
    }

    //============================================================================================================================================
    // Uncomment the line below to visualize signal, smoothed signal and trigger in Serial Plotter
    plot(detector->signalBuffer[detector->bufferIndex], irSmooth, trigger);
    //============================================================================================================================================

    decideCalmness(detector); // decide calmness state based on HR intervals

    // save signal values for next iteration
    irSmooth_prev = irSmooth;
    derivPrev = detector->signalState;

    // move index to next position
    detector->bufferIndex = (detector->bufferIndex + 1) % 50;

}

void decideCalmness(PeakDetectorState *detector) {
    if (detector->detectionState == 10) { //only check for calmness if finger is on and peak detection is initialized

        uint32_t hrIntervalSumLastTen = 0;
        uint32_t hrIntervalSumTheTenBefore = 0;
        uint8_t intervalsFound = 0;

        // Get last 10 intervals (or fewer if not yet available)
        for (uint8_t i = 0; i < 10; i++) {
            int j = detector->hrIntervalIndex - i;
            if (j < 0) j += 20;
            uint32_t val = detector->hrInterval[j];
            if (val != 0) { // array slot has been filled
                hrIntervalSumLastTen += val;
                intervalsFound++;
            }
        }
        // Get the 10 intervals before (or fewer if not yet available)
        for (uint8_t i = 10; i < 20; i++) {
            int j = detector->hrIntervalIndex - i;
            if (j < 0) j += 20;
            uint32_t val = detector->hrInterval[j];
            if (val != 0) { // array slot has been filled
                hrIntervalSumTheTenBefore += val;
                intervalsFound++;
            }
        }

        if (intervalsFound == 0) { //cannot happen due to detectionState but still just in case
            detector->chillVariable = 'u';
        } else {
            if(intervalsFound == 20){ //array fully filled -> calmness detection enabled
                uint32_t hrIntervalAvgTheTenBefore = (uint32_t) hrIntervalSumTheTenBefore / 10;
                uint32_t hrIntervalAvgLastTen = (uint32_t) hrIntervalSumLastTen / 10;

                    if (hrIntervalAvgTheTenBefore != 0) {
                        detector->hrRatio = (float)hrIntervalAvgLastTen / (float)hrIntervalAvgTheTenBefore;
                    } else {
                        detector->hrRatio = -1.0; //error value
                    }

                if (hrIntervalAvgLastTen >= (110U * hrIntervalAvgTheTenBefore) / 100U){
                    detector->chillVariable = 'c';
                    detector->calmUntilMillis = millis() + CALM_HOLD_MS;
                } else if ((int32_t)(detector->calmUntilMillis - (uint32_t)millis()) > 0 && detector->chillVariable == 'c') {
                    // still within hold window
                    detector->chillVariable = 'c';
                } else {
                    detector->chillVariable = 'u';
                } 
            }else{
                detector->hrRatio = 0.0; //not enough data yet
            }
            
        }
    } else {
        // reset hrInterval array when finger is off
        for (uint8_t i = 0; i < 20; i++) {
            detector->hrInterval[i] = 0;
        }
        //reset other variables
        detector->hrIntervalIndex = 0;
        detector->chillVariable = 'u';
        detector->hrRatio = 0.0;
        detector->calmUntilMillis = 0;
    }
    //===========================================================================================================================================
    // Uncomment the line below to print calmness debug info to Serial Monitor
    //printoutCalmnessDebug(detector);
    //===========================================================================================================================================
}

// Serial Plotter - HR Peak Detection - plot 3 signals
void plot(uint16_t val1, uint16_t val2, uint16_t val3) { //plot raw signal, smoothed signal, trigger
    Serial.print(val1);
    Serial.print("\t");
    Serial.print(val2);
    Serial.print("\t");
    Serial.print(val3);
    Serial.print("\n");
}

// Serial Monitor function - Calmness Detection - print debug info
void printoutCalmnessDebug(PeakDetectorState *detector) {
    Serial.print("DS: ");
    Serial.print(detector->detectionState);
    Serial.print("\t");
    Serial.print("Peak: ");
    Serial.print(detector->peakDetected);
    Serial.print("\t");
    Serial.print("Calmness: ");
    Serial.print(detector->chillVariable);
    Serial.print("\t");
    Serial.print("Idx: ");
    Serial.print(detector->hrIntervalIndex);
    Serial.print("\t");
    Serial.print("Ratio: ");
    Serial.print(detector->hrRatio, 3);
    Serial.print("\n");
}