#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include "peakDetectorState.h"
#include "flowerState.h"

void setupServo();
void testServo();
void testServoPeak();
void setSafeAngle(uint8_t angle);
void closeFlower(FlowerState* flower);
void openFlower(FlowerState* flower);
void handleFlower(FlowerState *flower, PeakDetectorState *detector, uint8_t peak);



#endif