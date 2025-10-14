// detectionState auf 1 => schwaches Licht
// Signal zum testen erstellen dass langsam von 0-4  durchtestet
// geschwindigkeit des Lichtpuls => relativ, richtig schnell, doppelt gleichzeitig
// funktion: startlightpulse: setzt aktuelle Leutdiode auf die erste. 
// keine Schleifen, in einer Variable speichern und dann zurück in die Funktion => sonst vergeht zu viel Zeit
// Fading einbauen

//start of code:

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "peakDetectorState.h"
#include "ledStrip.h"

// Create an instance of the Adafruit_NeoPixel class called 'strip' with the appropriate number of pixels, pin, and type
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int ledPosition = 0;
uint16_t LED_step_duration = 100; //how long it takes to swap from LED to LED in ms
uint16_t LED_fade_duration = 30; //how long the fading is in ms

// Activates the LEDs once per signal peak
void activateLEDsOnce(uint8_t reset_pos) {
  static uint8_t pos = 0;
  static uint32_t timestamp_nextLED = 0;
  static uint32_t timestamp_fade = 0;
  if (reset_pos) pos = 0;
  if(millis() - timestamp_fade > LED_fade_duration){ // For fading of LEDs 
    timestamp_fade = millis();
    for (int i = 0; i< LED_COUNT; i++)  {
      strip.getPixelColor(i);
      uint32_t color = strip.getPixelColor(i);
      uint8_t red = (color >> 16) & 0xFF;
      uint8_t green = (color >> 8) & 0xFF;
      uint8_t blue = color & 0xFF;
      red = float(red) * 0.9; // How much the LED is dimmed
      green = float(green) * 0.9;
      blue = float(blue) * 0.9;  
      strip.setPixelColor(i,red,green,blue); // Dims the pixel
    }
    strip.show();
  }

  if (millis() - timestamp_nextLED > LED_step_duration){ // For switching from LED to the next
    timestamp_nextLED = millis();
    if (pos <= LED_COUNT-1){
      strip.setPixelColor(pos, 50, 100, 150); // Pick a colour <3
      pos++;
      strip.show();
    }
  }
}

void controlLEDs(PeakDetectorState *detector) {
  if (detector -> detectionState >= 4) { // After the 4th peak this starts
      activateLEDsOnce(0);
  }
}
