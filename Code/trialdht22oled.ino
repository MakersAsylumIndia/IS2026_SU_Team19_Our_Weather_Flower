#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <DHT.h>             // DHT Sensor Library

// --- DISPLAY PIN CONFIGURATION ---
#define TFT_CS      5   
#define TFT_RST    22   
#define TFT_DC     21   
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// --- DHT22 CONFIGURATION ---
#define DHTPIN 14          
#define DHTTYPE DHT22      
DHT dht(DHTPIN, DHTTYPE);

// --- ULTRASONIC CONFIGURATION ---
#define TRIG_PIN 12
#define ECHO_PIN 13

// List of colors to cycle through every 10 seconds
const uint16_t colors[] = {
  ST7735_BLACK,
  ST7735_BLUE,
  tft.color565(50, 0, 50),   // Dark Purple
  tft.color565(0, 50, 20),   // Dark Green
  tft.color565(80, 40, 0)    // Brown/Orange
};
const int totalColors = sizeof(colors) / sizeof(colors[0]);

int colorIndex = 0;
unsigned long lastColorChange = 0;
const unsigned long colorInterval = 10000; // 10 seconds

void setup() {
  Serial.begin(115200);
  
  // Initialize Sensors
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize Screen
  tft.initR(INITR_BLACKTAB);   
  tft.setRotation(1); // Landscape
  tft.fillScreen(colors[colorIndex]);
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Handle background color swap every 10 seconds
  if (currentMillis - lastColorChange >= colorInterval) {
    lastColorChange = currentMillis;
    colorIndex = (colorIndex + 1) % totalColors;
    tft.fillScreen(colors[colorIndex]); 
  }

  // 2. Read DHT22 Data
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // 3. Clear the Trigger pin before firing
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5); 
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Read the bounce back
  long duration = pulseIn(ECHO_PIN, HIGH, 40000); // 40ms window
  float distance = duration * 0.0343 / 2.0;       

  // 4. Update the Display text
  tft.setTextSize(2);
  
  if (colors[colorIndex] == ST7735_BLUE || colors[colorIndex] == ST7735_BLACK) {
    tft.setTextColor(ST7735_WHITE, colors[colorIndex]); 
  } else {
    tft.setTextColor(ST7735_YELLOW, colors[colorIndex]);
  }

  // Header
  tft.setCursor(12, 10);
  tft.print("dakshatttttt");
  
  // Temperature
  tft.setCursor(12, 35);
  tft.print("Temp: ");
  if (!isnan(t)) { tft.print(t, 1); tft.print(" C "); } 
  else { tft.print("Err   "); }
  
  // Humidity
  tft.setCursor(12, 60);
  tft.print("Hum:  ");
  if (!isnan(h)) { tft.print(h, 1); tft.print(" % "); } 
  else { tft.print("Err   "); }

  // Distance (Ultrasonic)
  tft.setCursor(12, 85);
  tft.print("Dist: ");
  if (duration > 0 && distance >= 2.0 && distance <= 400.0) {
    tft.print(distance, 1);
    tft.print(" cm ");
  } else {
    tft.print("---   "); 
  }

  // Footer
  tft.setTextSize(1);
  tft.setCursor(12, 112);
  tft.print("sir i am working!");

  delay(500); 
}