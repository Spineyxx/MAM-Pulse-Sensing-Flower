#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "readSensor.h"
#include "peakDetectorState.h"
#include "MAX30105.h"           // library: Sparkfun MAX3010x
#include "leds.h"
#include "detectPeaks.h"


// Create an instance of the Adafruit_NeoPixel class called 'strip' with the appropriate number of pixels, pin, and type
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// für getActColor
uint32_t colorSequence[] = { RED, YELLOW, CYAN, PURPLE };
int colorSequenceLength = sizeof(colorSequence) / sizeof(colorSequence[0]);
//

// für stripPulseMulti
//struct to track each pulse
struct Sweep { 
    uint8_t pos;
    uint32_t color;
    bool active;
};

Sweep sweeps[MAX_SWEEPS];   // tracks all active pulses
uint32_t lastSweepTimestamp = 0;  // timestamp for pulse updates
bool ringBlockAlwaysOn = false;
//

// for getHRColor
const int hrMin = 60;   // blue at this value
const int hrMax = 120;  // red at this value
//



// setup
void setupStripPulse() {
  // initialize LED strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'  
}


// dim all Leds by dimFactor, keep minimum brightness if keepStandbyBrightness is true
void dimLeds(float dimFactor, uint8_t keepStandbyBrightness) {
  for (int i = 0; i < LED_COUNT; i++) {
    uint32_t c = strip.getPixelColor(i);
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;
    r *= dimFactor;
    g *= dimFactor;
    b *= dimFactor;
    if (keepStandbyBrightness) {
      if (i < LED_COUNT -7) { //= first 16 LEDs = LED Strip
        // keep a bit of glow for standby, this makes purple/magenta:
        if (r < 3) r = 3;
        if (g < 0) g = 0;
        if (b < 3) b = 3;
      } else {
        // last 7 LEDs = LED Ring, this makes white:
        if (r < 3) r = 3;
        if (g < 3) g = 3;
        if (b < 3) b = 3;
      }
      
    }
    strip.setPixelColor(i, r, g, b);
  }
}

uint32_t fadeColor(uint32_t col1, uint32_t col2, int progress) {
// progress from 0 to COLORS_FADE_STEPS
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

uint32_t getActColor() { // goes through fixed sequence of colors
  static int sequenceState = 0;
  static int colorProgress = 0;

  uint32_t colorFrom, colorTo;
  if (colorProgress >= COLORS_FADE_STEPS) {
    colorProgress = 0;
    sequenceState = (sequenceState + 1) % colorSequenceLength;
  }
  colorFrom = colorSequence[sequenceState];
  colorTo = colorSequence[(sequenceState + 1) % colorSequenceLength];
  uint32_t c = fadeColor(colorFrom, colorTo, colorProgress);
  colorProgress++;
  return c;
}



// Map current HR to a color from blue to green to yellow to orange to red
uint32_t getHRColor(PeakDetectorState *detector) { 
  
  // Clamp currentHRAverage to range
  if (detector->currentHRAverage <= hrMin) detector->currentHRAverage = hrMin;
  if (detector->currentHRAverage >= hrMax) detector->currentHRAverage = hrMax;

  // Define colors in the gradient
  uint32_t colors[] = {
    0x0000FF, // Blue
    0x00FF00, // Green
    0xFFFF00, // Yellow
    0xFFA500, // Orange
    0xFF0000  // Red
  };
  int nColors = sizeof(colors) / sizeof(colors[0]);

  // Map currentHRAverage to position along the gradient
  float fraction = float(detector->currentHRAverage - hrMin) / float(hrMax - hrMin);
  float pos = fraction * (nColors - 1);
  int idx1 = int(pos);
  int idx2 = min(idx1 + 1, nColors - 1);
  int progress = int((pos - idx1) * COLORS_FADE_STEPS);

  return fadeColor(colors[idx1], colors[idx2], progress); // fading of colors
}

// Pulses using getActColor for color sequence, goes through fixed sequence
void stripPulse(PeakDetectorState *detector) {
  static uint8_t pos = 0;
  static uint32_t timestamp = 0;
  static uint8_t active = 0;
  
  if (sensorAvailable == 1) {
    if(detector->detectionState > 0){//if (amplitude > MIN_ACTIVE_AMPLITUDE){ 
      if (detector->peakDetected == 1){
        pos = 0;
        active = 2; // state when peak is detected
      } else if (active != 2){
        active = 1; // state when sensor is active
      }
    } else { 
      active = 0; // state when sensor is inactive
      pos = 0; } 
  }

  int minStep = 3;   // very fast movement at high HR
  int maxStep = 20;  // slow movement at low HR

  float hrFraction = float(detector->currentHRAverage - hrMin) / float(hrMax - hrMin);
  hrFraction = constrain(hrFraction, 0.0, 1.0);

  int stepTime = maxStep - hrFraction * (maxStep - minStep);

  if (millis() - timestamp > stepTime) { 
    timestamp = millis();
    dimLeds(DIMFACTOR, active);
    if (active == 2){
      if (pos <= LED_COUNT) {
        uint32_t color = getHRColor(detector); // choose either getActColor() for sequence or getHRColor() for HR dependable color
        strip.setPixelColor(LED_COUNT-pos, color);
        pos++;
      }else{
        active = 1;
      }
      
    }
    strip.show();
  }
}


void stripPulseMulti(PeakDetectorState *detector) {

    // Start a new sweep if a peak is detected
    if (sensorAvailable == 1 && detector->detectionState > 0 && detector->peakDetected == 1) {

        ringBlockAlwaysOn = true;

        for (int i = 0; i < MAX_SWEEPS; i++) {
            if (!sweeps[i].active) {
                sweeps[i].active = true;
                sweeps[i].pos = 0;
                sweeps[i].color = getHRColor(detector);
                break; // only start one new sweep per peak
            }
        }
    }

    // Turn off ring if finger is removed
    if (!(sensorAvailable == 1 && detector->detectionState > 0)) {
        ringBlockAlwaysOn = false;
    }

    int minStep = 3; // very fast at high HR
    int maxStep = 20; // slow at low HR

    float hrFraction = float(detector->currentHRAverage - hrMin) / float(hrMax - hrMin);
    hrFraction = constrain(hrFraction, 0.0, 1.0);

    int stepTime = maxStep - hrFraction * (maxStep - minStep);

    // see if it's time to update LEDs
    if (millis() - lastSweepTimestamp > stepTime) {
        lastSweepTimestamp = millis();

        bool keepGlow = (sensorAvailable == 1 && detector->detectionState > 0);
        dimLeds(DIMFACTOR, keepGlow ? 1 : 0);

        // Compute HR color, needed later again
        uint32_t hrColor = getHRColor(detector);

        // update sweeps
        for (int i = 0; i < MAX_SWEEPS; i++) {
            if (sweeps[i].active) {

                if (sweeps[i].pos < LED_COUNT_STRIP) {
                    // LED Strip
                    strip.setPixelColor(LED_COUNT_STRIP - 1 - sweeps[i].pos, sweeps[i].color);
                }
                else if (sweeps[i].pos == LED_COUNT - 1) {
                    // Center ring
                    strip.setPixelColor(LED_COUNT - 1, sweeps[i].color);
                }
                else if (sweeps[i].pos < LED_COUNT - 1) {
                    // Outer ring
                    for (int j = LED_COUNT_STRIP; j < LED_COUNT; j++) {
                        strip.setPixelColor(j, sweeps[i].color);
                    }
                }
                else {
                    sweeps[i].active = false;
                }

                sweeps[i].pos++;
            }
        }

        // to keep light on outside of the ring on constantly
        if (ringBlockAlwaysOn) {
            for (int led = LED_COUNT_STRIP + 1; led < LED_COUNT; led++) {
                strip.setPixelColor(led, hrColor);
            }
        }

        strip.show();
    }
}


// For testing of hardware
// Funktion, die alle LEDs rot leuchten lässt, zum Testen ob der LED Strip funktioniert
void functiontestLEDSTrip() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0)); // Setzt jede LED auf rot
  }
  strip.show();  // Wendet die Änderungen an
}

// LED Ring functions
void ledRingControl() {
    // LED indices for the last 7 LEDs
    for (int i = 16; i < 23; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255)); // White
    }
    strip.show();
}
//


// old functions
// für activateLEDsOnce
int ledPosition = 0;
uint16_t LED_step_duration = 30; //how long it takes to swap from LED to LED in ms   100  25
uint16_t LED_fade_duration = 10; //how often a fade step is performed                30   10
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

// LED RING
// für RING
static bool ringActive = false;
static uint32_t ringStartTime = 0;
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