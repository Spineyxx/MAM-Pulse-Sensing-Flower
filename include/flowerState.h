#ifndef FLOWERSTATE_H
#define FLOWERSTATE_H

#include <Arduino.h>

//Degree LIMITS - operational range for the flower servo - changing these could lead to mechanical issues!
#define FLOWER_OPEN_ANGLE 45 //the servo angle for fully opened flower - NUMBER IS CURRENTLY A FILLER
#define FLOWER_CLOSED_ANGLE 0 //the servo angle for fully closed flower - NUMBER IS CURRENTLY A FILLER

typedef struct {
    uint8_t currentAngle;
    uint8_t servoCounter;
    uint8_t motion; //0 = closed, 1 = opening, 2 = open, 3 = closing
    uint32_t lastMotionTimestamp;
} FlowerState;

#endif