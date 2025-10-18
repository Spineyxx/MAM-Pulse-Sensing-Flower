#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include "detectPeaks.h"
#include "ledStrip.h"
#include "peakDetectorState.h"
#include "readSensor.h"

PeakDetectorState detector = {.signalBuffer = {0},
                              .bufferIndex = 0,
                              .lastPeak = 0,
                              .peakBeforeLast = 0,
                              .signalState = 'r',
                              .hrInterval = {0},
                              .hrIntervalIndex = 0,
                              .detectionState = 0};


// the setup function is called once (when the program starts)
void setup() {
    Serial.begin(115200);
    //testStripSetup(); // zum Testen der LEDs nötig ~LINDA
    setupSensor();  // zum Testen des Sensors nötig ~ANNA
}

// after the setup() function returned, the loop function is called in an endless loop
void loop() {
    getPulseOxySignal(&detector);  // zum Testen des Sensors nötig ~ANNA
    int peak = detectPeaks(&detector);
    //Serial.println(peak);
    //testStripLoop(&detector); // zum Testen der LEDs nötig ~LINDA
}

// NOTES ON DATA TYPES:
// uint8_t für Werte von 0-255
// uint16_t für Werte von 0-65535
// uint32_t für Werte von 0-4.294.967.295
// int16_t für Werte von -32.768 bis 32.767
// int32_t für Werte von -2.147.483.648 bis 2.

//=========================== CHAT ========================================
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
//@ANNA: Du bisch so cool
//@LINDA: dine dialekt impression isch wieda on point ;) I han btw da
// array mit da letschten 5 HR intervalle in da struct inegspeichert:
// detector->hrInterval[hrIntervalIndex]
// ^ dean bruchscht du wahrscheinlich, domit du adaptiera kannsch wia 
// schnell sich LEDs fortbewegan, wenn die Intervalle kürzer werden
//=========================================================================
