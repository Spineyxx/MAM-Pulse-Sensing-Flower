/*
  Streaming of sound data with Bluetooth to other Bluetooth device.
  We can build a list of available devices and connect to a specific one,
  or connnect to the closest device.

  The sound data is provided by a callback function in frames of 20ms.  
  To create the const arrays for sample data, used the provided mp3_to_header.py script.
  Compile with partition scheme "huge app" - see platformio.ini

  This code is based upon the A2DP library by Phil Schatzmann,
  see: https://github.com/pschatzmann/ESP32-A2DP
  https://github.com/pschatzmann/ESP32-A2DP/wiki/PlatformIO

  Copyright (C) 2020 Phil Schatzmann
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "btSound.h"
#include <Arduino.h>
#include "BluetoothA2DPSource.h"
#include "SoundUtils.h"
#include <ESP32PWM.h>

//Sound data
#include "heartBeatOnce.h"

//================================================================================================
// INSERT THE NAME OF YOUR BLUETOOTH SPEAKER/HEADPHONES HERE
const char* soundboxSSID = "NSVE S8 PRO"; //e.g. "SoundCore mini" is for Anker Soundcore Mini
// If you are unsure about the name, uncomment the Serial prints in the isValid() function
//================================================================================================

BluetoothA2DPSource a2dp_source;
SoundData beatSound((int16_t*)heartBeatOnce_raw, heartBeatOnce_raw_len/2); 
SoundMixer mixer{&beatSound};

int32_t get_data_frames(Frame *frame, int32_t frame_count) {
    static uint32_t callbackCount = 0;
    callbackCount++;
    for (int sample = 0; sample < frame_count; ++sample) {
        if (beatSound.playing && (int)beatSound.pos < beatSound.len) {
            int16_t value = beatSound.data[(int)beatSound.pos];
            frame[sample].channel1 = value;
            frame[sample].channel2 = value;
            beatSound.pos += beatSound.playbackSpeedFactor;
        } else {
            // Not playing: output silence
            frame[sample].channel1 = 0;
            frame[sample].channel2 = 0;
        }
    }
    yield(); // to prevent watchdog
    return frame_count;
}

// Callback for valid BT audio connections (in pairing mode)
// return true in order to connect to the device
bool isValid(const char* ssid, esp_bd_addr_t address, int rssi){
   //Serial.printf("Found available SSID: %s, RSSI: %d\n", ssid, rssi);
   if (strcmp(ssid, soundboxSSID) == 0){
        //Serial.printf("Connecting to %s", soundboxSSID);
        return true;
   }else{
        return false;
   }
}

void setupBT(){
  // Use LEDC timer 4 (less likely to clash with other libs)
  ESP32PWM::allocateTimer(4);
  a2dp_source.set_ssid_callback(isValid);
  // a2dp_source.set_auto_reconnect(false);
  // a2dp_source.set_volume(80);
  a2dp_source.set_data_callback_in_frames(get_data_frames);    // feed data to the a2dp source
  a2dp_source.start(); 
}


void handleBT(FlowerState *flower, PeakDetectorState *detector){
    static bool wasConnected = false;
    bool connected = a2dp_source.is_connected();
    if (connected && !wasConnected) {
        //Serial.println("Bluetooth connected.");
    }
    // Peak detected: start beat sound
    if (connected && detector->peakDetected == 1) {
        int idx = (int)detector->hrIntervalIndex-(int)1;
        if (idx < 0) idx += 20;
        uint8_t index = (uint8_t) idx;
        uint32_t interval = detector->hrInterval[index];
        //set speed of playback depending on HR interval
        if (interval > 0) {
            float playbackSpeed = (float)beatSound.len /((float)interval*10);
            if (playbackSpeed < 0.1f) playbackSpeed = 0.1f; // avoid zero or negative
            beatSound.playbackSpeedFactor = playbackSpeed;
        } else {
            beatSound.playbackSpeedFactor = 1.0f;
        }
        beatSound.pos = 0;
        beatSound.play();
    }
    // Stop beat when finished
    if (beatSound.playing && (int)beatSound.pos >= beatSound.len) {
        beatSound.stop();
    }
    if (connected) {
        mixer.advancePositions();
    }
    wasConnected = connected;
}