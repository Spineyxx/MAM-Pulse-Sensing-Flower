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
#include "music.h"

BluetoothA2DPSource a2dp_source;
SoundData musicSound((int16_t*)music_raw, music_raw_len/2); 
SoundMixer mixer{&musicSound};

int32_t get_data_frames(Frame *frame, int32_t frame_count) {
    static uint32_t callbackCount = 0;
    callbackCount++;
    Serial.print("A2DP callback #");
    Serial.print(callbackCount);
    Serial.print(" | playing: ");
    Serial.print(musicSound.playing);
    Serial.print(" | pos: ");
    Serial.print(musicSound.pos);
    Serial.print(" / ");
    Serial.print(musicSound.len);
    Serial.print("\n");
    for (int sample = 0; sample < frame_count; ++sample) {
        if (musicSound.playing && (int)musicSound.pos < musicSound.len) {
            int16_t value = musicSound.data[(int)musicSound.pos];
            frame[sample].channel1 = value;
            frame[sample].channel2 = value;
            musicSound.pos += musicSound.playbackSpeedFactor;
        } else {
            // Loop: restart playback from beginning
            musicSound.pos = 0;
            musicSound.playing = true;
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
   Serial.printf("Found available SSID: %s, RSSI: %d\n", ssid, rssi);
   if (strcmp(ssid, "SoundCore mini") == 0){
        Serial.println("Connecting to SoundCore mini");
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


void handleBT(FlowerState *flower){
    static bool wasConnected = false;
    bool connected = a2dp_source.is_connected();

    if (connected && !wasConnected) {
        Serial.println("Bluetooth connected: starting playback from beginning.");
        musicSound.play();
        Serial.print("musicSound.play() called. playing: ");
        Serial.print(musicSound.playing);
        Serial.print(" | pos: ");
        Serial.print(musicSound.pos);
        Serial.print(" / ");
        Serial.print(musicSound.len);
        Serial.print("\n");
    } else if (connected) {
        mixer.advancePositions();
        Serial.print("BT still connected. mixer.advancePositions() called. pos: ");
        Serial.print(musicSound.pos);
        Serial.print(" / ");
        Serial.print(musicSound.len);
        Serial.print("\n");
    }
    wasConnected = connected;

    
}