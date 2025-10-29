#ifndef SERVO_H
#define SERVO_H
#include <Arduino.h>
#include "peakDetectorState.h"

void setupServo();
void setServoAngle(int angle);
void testServo();
void testServoPeak();


#endif