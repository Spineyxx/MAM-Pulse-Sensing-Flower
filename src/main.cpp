#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include "detectPeaks.h"
#include "ledStrip.h"
#include "peakDetectorState.h"
#include "readSensor.h"

PeakDetectorState detector = {.signalBuffer = {0},
                              .bufferIndex = 0,
                              .lastPeak = 0,
                              .signalState = 'r',
                              .detectionState = 0};

// the setup function is called once (when the program starts)
void setup() {
    Serial.begin(115200);
    // strip.begin();
    // for (int i = 0; i < LED_COUNT; i++)
    //{
    //   strip.setPixelColor(i, 0, 0, 0);
    // }
    // strip.show();
    setupSensor();  // damit ohne Pulssensor getestet werden kann ~LINDA
}

// after the setup() function returned, the loop function is called in an
// endless loop
void loop() {
    getPulseOxySignal(&detector);  // damit ohne Pulssensor getestet werden kann ~LINDA
    int peak = detectPeaks(&detector);  // damit ohne Pulssensor getestet werden kann ~LINDA
    Serial.println(peak);

    //=========================================================================
    //@LINDA: detectPeaks(&detector) returnt "1" wenn ein Peak erkannt wurde,
    // sonst "0"
    // in Zeile 31 save ich das Ergebnis als Beispiel in die Variable "peak"
    // hinein und printe die dann über den Seriellen Monitor aus (Zeile 32).
    // Dann spuckt es jedes Mal eine 1 aus, wenn ein Peak erkannt wurde, sonst
    // die 0 (das war nur zum Testen)
    //--> Mit Veigl haben wir eine Funktion geschrieben, die bei jedem Peak die LEDs
    //    einmal aufleuchten lässt. Die Funktion heißt "activateLEDsOnce(1);"
    //    und wenn in der Klammer eine 1 steht, leuchten die LEDs einmal auf, bei einer 0 passiert
    // nichts. Wenn man jetzt activateLEDsOnce(detectPeaks(&detector));
    // schreibt, dann leuchten die LEDs immer dann auf, wenn ein Peak erkannt
    // wurde, weil dann ist detectPeaks(&detector) = 1 und activateLEDsOnce(1);
    // --> LEDs leuchten auf
    //=========================================================================

    // detector.detectionState = 4; // // damit ohne Pulssensor getestet werden
    // kann ~LINDA static uint32_t ts = 0;      // // damit ohne Pulssensor
    // getestet werden kann ~LINDA if (millis() - ts > 1000)
    //{ //                                                 // damit ohne
    // Pulssensor getestet werden kann ~LINDA
    //  ts = millis();
    //  activateLEDsOnce(1); // resettet pos
    //}
    // controlLEDs(&detector); // Function for controlling the LED Strip
}

// NOTES ON DATA TYPES:
// uint8_t für Werte von 0-255
// uint16_t für Werte von 0-65535
// uint32_t für Werte von 0-4.294.967.295
// int16_t für Werte von -32.768 bis 32.767
// int32_t für Werte von -2.147.483.648 bis 2.
