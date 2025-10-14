#include "detectPeaks.h"


void detectPeaks(PeakDetectorState* detector){
  static uint32_t potentialPeak = 0;
  static unsigned long hrInterval = 0;
  static char derivPrev = 'r'; //r = rising; f = falling
  static int irSmooth_prev = 0;
  static int trigger = 0;
  static boolean triggered = false;
  static int triggerOld = 0;
  static boolean triggeredOld = false;


  //Average of last 5 samples (irSmooth) and last 50 samples (irBaseline):
  long sumSmooth = 0;
  long sumBaseline = 0;


  for (int i = 0; i < 5; i++) { //take last 5 of the array that holds last 50
    int index = ((detector->bufferIndex - i + 50) % 50); //to not go beyond bounds of array
    sumSmooth += detector->signalBuffer[index];
  }
  long irSmooth = sumSmooth / 5; //=average of last 5 values

  for (int i = 0; i < 50; i++) { 
    sumBaseline += detector->signalBuffer[i]; 
  }
  long irBaseline = sumBaseline / 50; //=average of last 50 values
  //note: calculation errors until the array is filled with respective amount of values
  
  
  
  if(irBaseline > 10000){ //FINGER ON = TRUE --> start Peak Detection
    if(detector->detectionState == 0){detector->detectionState++;}

    //CURRENT PEAK DETECTION
    if(irSmooth >= irSmooth_prev){ //if current value is higher than the previous
      detector->signalState = 'r'; //signal is rising
    }else{
      detector->signalState = 'f'; //else it is falling
    }

    if(derivPrev == 'r' && detector->signalState == 'f'){ //if it used to rise, but now falls
        potentialPeak = millis(); //get timestamp of potential peak   
        
        switch(detector->detectionState){
            case 1: //finger just placed onto the sensor, first 3 peaks are accepted
            case 2:
            case 3:
                detector->thePeakBefore = detector->lastPeak;  
                detector->lastPeak = potentialPeak;
                hrInterval = 200;  
                detector->detectionState++;
                break;
            case 4: //from now on check interval
                unsigned long diff = potentialPeak - detector->lastPeak;
                if(diff > hrInterval*0.6){
                    detector->thePeakBefore = detector->lastPeak;  
                    detector->lastPeak = potentialPeak;
                    hrInterval = diff;

                    if(triggered){
                        trigger = irBaseline - 200;
                        triggered = false;
                    }else{
                        trigger = irBaseline + 200;
                        triggered = true;
                    }

                }
                break;
        }

        if(triggeredOld){
        triggerOld = irBaseline - 200;
        triggeredOld = false;
        }else{
        triggerOld = irBaseline + 200;
        triggeredOld = true;
        }

        }
      } else {
    detector->detectionState = 0;    
  }

  plot(detector->signalBuffer[detector->bufferIndex], irSmooth, trigger, triggerOld);

  //save signal values for next iteration
  irSmooth_prev = irSmooth;
  derivPrev = detector->signalState;

  //move index to next position
  detector->bufferIndex = (detector->bufferIndex + 1) % 50;
};

//Serial Plotter - HR Peak Detection
void plot(int amplitude, int irSmooth, int trigger, int triggerOld){
    Serial.print(amplitude);
    Serial.print("\t");
    Serial.print(irSmooth); 
    Serial.print("\t");
    //Serial.print(irBaseline); 
    //Serial.print("\t");
    Serial.print(trigger);
    Serial.print("\t");
    Serial.print(triggerOld);
    Serial.print("\n");
}