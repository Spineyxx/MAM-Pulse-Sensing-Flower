#include "readSensor.h"

#include "MAX30105.h"  // library: Sparkfun MAX3010x

MAX30105 particleSensor;

// SENSOR CONNECTIONS:
// VIN to 3.3V
// GND to GND
// SCL to ESP32 SCL (GPIO22) - unless pin is changed in Wire.begin
// SDA to ESP32 SDA (GPIO21) - unless pin is changed in Wire.begin

void setupSensor() { //Initialization of sensor
    Wire.begin(26,27); //changing I2C pins to 26 and 27
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST))  // 400kHz speed
    {
        Serial.println("MAX30105 was not found. Please check wiring/power. ");
        while (1);
    }

    // Setup to sense a nice looking saw tooth on the plotter
    byte ledBrightness = 0x1F;  // Options: 0=Off to 255=50mA
    byte sampleAverage = 8;     // Options: 1, 2, 4, 8, 16, 32
    byte ledMode = 3;  // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
    int sampleRate = 400;  // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
    int pulseWidth = 411;  // Options: 69, 118, 215, 411
    int adcRange = 4096;   // Options: 2048, 4096, 8192, 16384

    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate,
                         pulseWidth,
                         adcRange);  // Configure sensor with these settings

    
    //===========================================================================
    // SAMPLE CODE - UP FOR DELETION WHEN WORK IS DONE
    //---------------------------------------------------------------------------
    // Arduino plotter auto-scales annoyingly. To get around this, pre-populate
    // the plotter with 500 of an average reading from the sensor
    // Take an average of IR readings at power up
    // const byte avgAmount = 64;
    // long baseValue = 0;
    // for (byte x = 0 ; x < avgAmount ; x++)
    //{
    //   baseValue += particleSensor.getIR(); //Read the IR value
    // }
    // baseValue /= avgAmount;
    // Pre-populate the plotter so that the Y scale is close to IR values
    // for (int x = 0 ; x < 500 ; x++)
    //   Serial.println(baseValue);
    //===========================================================================
};

void getPulseOxySignal(PeakDetectorState *detector) {
    detector->signalBuffer[detector->bufferIndex] = particleSensor.getIR();
}