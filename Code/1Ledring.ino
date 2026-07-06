#include <Adafruit_NeoPixel.h>

#define LED_PIN    13  // The ESP32 pin connected to the NeoPixel DIN
#define NUM_LEDS    8  // Number of LEDs in your ring

// Initialize the NeoPixel library
Adafruit_NeoPixel ring(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  ring.begin();           // INITIALIZE NeoPixel ring object
  ring.show();            // Turn OFF all pixels ASAP
  ring.setBrightness(50); // Set brightness to ~20% (saves eyes and power!)
}

void loop() {
  // Cycle through Red, Green, and Blue
  colorWipe(ring.Color(255, 0, 0), 100); // Red
  colorWipe(ring.Color(0, 255, 0), 100); // Green
  colorWipe(ring.Color(0, 0, 255), 100); // Blue
}

// Function to fill the dots one after the other with a color
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<ring.numPixels(); i++) {
    ring.setPixelColor(i, color);
    ring.show();
    delay(wait);
  }
}