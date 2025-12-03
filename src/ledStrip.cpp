//Notes:

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "readSensor.h"
#include "peakDetectorState.h"
#include "MAX30105.h"           // library: Sparkfun MAX3010x
#include "ledStrip.h"
#include "detectPeaks.h"


// Create an instance of the Adafruit_NeoPixel class called 'strip' with the appropriate number of pixels, pin, and type
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// für activateLEDsOnce
int ledPosition = 0;
uint16_t LED_step_duration = 30; //how long it takes to swap from LED to LED in ms   100  25
uint16_t LED_fade_duration = 10; //how often a fade step is performed                30   10

// für INI
uint32_t colorSequence[] = { RED, YELLOW, CYAN, PURPLE };
int colorSequenceLength = sizeof(colorSequence) / sizeof(colorSequence[0]);
//uint8_t sensorAvailable = 0;

// für RING
static bool ringActive = false;
static uint32_t ringStartTime = 0;



// INI (= nur der Strip)
// dim all Leds by dimFactor, keep minimum brightness if keepStandbyBrightness is true
void INI_dimLeds(float dimFactor, uint8_t keepStandbyBrightness) {
  for (int i = 0; i < LED_COUNT_STRIP; i++) {
    uint32_t c = strip.getPixelColor(i);
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;
    r *= dimFactor;
    g *= dimFactor;
    b *= dimFactor;
    if (keepStandbyBrightness) {
      // keep a bit of orange glow for standby
      if (r < 3) r = 3;
      if (g < 1) g = 1;
    }
    strip.setPixelColor(i, r, g, b);
  }
}

uint32_t INI_fadeColor(uint32_t col1, uint32_t col2, int progress) {
// note: progress: 0..COLORS_FADE_STEPS
    uint8_t r1 = (col1 >> 16) & 0xFF;
    uint8_t g1 = (col1 >> 8) & 0xFF;
    uint8_t b1 = col1 & 0xFF; 
    uint8_t r2 = (col2 >> 16) & 0xFF;
    uint8_t g2 = (col2 >> 8) & 0xFF;
    uint8_t b2 = col2 & 0xFF;
    uint8_t red = (((r2 * progress) + (r1 * (COLORS_FADE_STEPS - progress))) / COLORS_FADE_STEPS);
    uint8_t green = (((g2 * progress) + (g1 * (COLORS_FADE_STEPS - progress))) / COLORS_FADE_STEPS);
    uint8_t blue = (((b2 * progress) + (b1 * (COLORS_FADE_STEPS - progress))) / COLORS_FADE_STEPS);
    return (red << 16) | (green << 8) | blue;
}

uint32_t INI_getActColor() {
  static int sequenceState = 0;
  static int colorProgress = 0;

  uint32_t colorFrom, colorTo;
  if (colorProgress >= COLORS_FADE_STEPS) {
    colorProgress = 0;
    sequenceState = (sequenceState + 1) % colorSequenceLength;
  }
  colorFrom = colorSequence[sequenceState];
  colorTo = colorSequence[(sequenceState + 1) % colorSequenceLength];
  uint32_t c = INI_fadeColor(colorFrom, colorTo, colorProgress);
  colorProgress++;
  return c;
}

void setupStripPulse() {
  // initialize sensor LED strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'  
}

void strip_pulse(PeakDetectorState *detector) {
  static uint8_t pos = 0;
  static uint32_t timestamp = 0;
  static uint8_t active = 0;
  
  if (sensorAvailable == 1) {
    //int amplitude = particleSensor.getIR();   //nicht nötig, weil detectionState schon existiert
    if(detector->detectionState > 0){//if (amplitude > MIN_ACTIVE_AMPLITUDE){ 
      if (detector->peakDetected == 1){
        //Serial.println("PEAK!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        pos = 0;
        active = 2; // state when peak is detected
      } else if (active != 2){
        active = 1; // state when sensor is active
      }
    } else { 
      active = 0; // state when sensor is inactive
      pos = 0; } 
  }

  if (millis() - timestamp > LED_STEP_DURATION_INI) { 
    timestamp = millis();
    INI_dimLeds(DIMFACTOR, active);
    if (active == 2){
      if (pos <= LED_COUNT_STRIP) {
        uint32_t color=INI_getActColor();
        strip.setPixelColor(LED_COUNT_STRIP-pos, color);
        pos++;
      }else{
        active = 1;
      }
      
    }
    strip.show();
  }
}



// LED RING
void pulseRing() {
  // Activate ring if not already active
  if (!ringActive) {
    ringActive = true;
    ringStartTime = millis();

    // Light up all ring LEDs
    for (int i = LED_COUNT_STRIP; i < LED_COUNT; i++) {
      strip.setPixelColor(i, RING_COLOR);
    }
    strip.show();
  }
}

void updateRing() {
  if (ringActive && millis() - ringStartTime >= RING_DURATION) {
    // Turn off ring LEDs
    for (int i = LED_COUNT_STRIP; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 0); // off
    }
    strip.show();
    ringActive = false;
  }
}

void loopRING(PeakDetectorState *detector){
  if(detector->peakDetected == 1){
    pulseRing();
  }
  updateRing();
}









///Testen
// Funktion, die alle LEDs rot leuchten lässt, zum Testen ob der LED Strip funktioniert
void functiontestLEDSTrip() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0)); // Setzt jede LED auf rot
  }
  strip.show();  // Wendet die Änderungen an
}

//ALT
// Activates the LEDs once per signal peak
void activateLEDsOnce(uint8_t reset_pos) { 
  static uint8_t pos = 0;
  static uint32_t timestamp_nextLED = 0;
  static uint32_t timestamp_fade = 0;
  if (reset_pos) pos = 0;
  if(millis() - timestamp_fade > LED_fade_duration){ // For fading of LEDs 
    timestamp_fade = millis();
    for (int i = 0; i< LED_COUNT_STRIP; i++)  {
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
    if (pos <= LED_COUNT_STRIP){
      strip.setPixelColor(LED_COUNT_STRIP - pos, 200, 150, 200); // Pick a colour <3
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


void testStripSetup(){
  strip.begin();
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
}


void testStripLoop(PeakDetectorState *detector){
  detector -> detectionState = 4; //                                              
  static uint32_t ts = 0;      //                                                   
  if (millis() - ts > 900)
  { //                                                 
    ts = millis();
    activateLEDsOnce(1); // resettet pos
  }
  controlLEDs(detector); // Function for controlling the LED Strip
}