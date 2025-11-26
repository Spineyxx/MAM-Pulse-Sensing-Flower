#ifndef BTSOUND_H
#define BTSOUND_H
#include <BluetoothA2DPSource.h>
#include <Arduino.h>
#include "BluetoothA2DPSource.h"
#include "SoundUtils.h"
#include <ESP32PWM.h>
#include "flowerState.h"

int32_t get_data_frames(Frame *frame, int32_t frame_count);
bool isValid(const char* ssid, esp_bd_addr_t address, int rssi);
void setupBT();
void handleBT(FlowerState *flower);

#endif 