#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include "peakDetectorState.h"
#include "flowerState.h"

void setupServo();
void testServo();
void testServoPeak();
void setSafeAngle(uint8_t angle);
void toggleFlower(FlowerState *flower);


#endif