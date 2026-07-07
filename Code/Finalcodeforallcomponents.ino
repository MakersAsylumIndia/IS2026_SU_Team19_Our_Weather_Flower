#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <DHT.h>             // DHT Sensor Library
#include <Adafruit_NeoPixel.h> // NeoPixel Ring Library
#include <ESP32Servo.h>      // Hardware-specific Servo library for ESP32

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

// --- RGB RING CONFIGURATION ---
#define LED_PIN     4   
#define NUM_LEDS    8   
Adafruit_NeoPixel ring(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- TOUCH SENSOR CONFIGURATION ---
#define TOUCH_PIN  27  

// --- MQ-135 GAS SENSOR CONFIGURATION ---
#define MQ135_PIN  34  

// --- SERVO CONFIGURATION ---
#define SERVO_PIN   2  
Servo myServo;

// List of display backgrounds
const uint16_t displayColors[] = {
  ST7735_BLACK,
  ST7735_BLUE,
  tft.color565(50, 0, 50),   // Dark Purple
  tft.color565(0, 50, 20),   // Dark Green
  tft.color565(80, 40, 0)    // Brown/Orange
};
const int totalDisplayColors = sizeof(displayColors) / sizeof(displayColors[0]);

const char* colorNames[] = {
  "BLACK",
  "BLUE",
  "PURPLE",
  "GREEN",
  "BROWN"
};

int displayColorIndex = 0;
unsigned long lastColorChange = 0;
const unsigned long colorInterval = 10000; 

// --- RGB RING COLOR CYCLE LOGIC ---
uint32_t ringColors[] = {
  ring.Color(0, 255, 0),    // Green
  ring.Color(255, 0, 0),    // Red
  ring.Color(255, 255, 0),  // Yellow
  ring.Color(0, 0, 255),    // Blue
  ring.Color(255, 0, 255),  // Purple
  ring.Color(0, 255, 255)   // Cyan
};
const int totalRingColors = sizeof(ringColors) / sizeof(ringColors[0]);
int ringColorIndex = 0; 

int lastTouchState = LOW; 

void setup() {
  Serial.begin(115200);
  
  // Initialize Hardware
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TOUCH_PIN, INPUT); 
  
  // Initialize Servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myServo.setPeriodHertz(50);    
  myServo.attach(SERVO_PIN, 500, 2400); 
  myServo.write(0);              
  
  ring.begin();
  ring.setBrightness(60); 
  ring.show();            

  // Initialize Screen
  tft.initR(INITR_BLACKTAB);   
  tft.setRotation(1); 
  tft.fillScreen(displayColors[displayColorIndex]);
}

void loop() {
  unsigned long currentMillis = millis();
  int currentTouchState = digitalRead(TOUCH_PIN);

  // 1. Read DHT22 Data
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // 2. Read Ultrasonic Sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5); 
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 40000); 
  float distance = duration * 0.0343 / 2.0;       

  // 3. Read MQ-135 Gas Sensor
  int gasRaw = analogRead(MQ135_PIN);

  // Check conditions: Is someone closer than 20cm?
  bool isSomeoneNear = (duration > 0 && distance >= 2.0 && distance <= 20.0);

  // 4. COLOR SWAPPING LOGIC (Triggered by Touch)
  if (currentTouchState == HIGH && lastTouchState == LOW) {
    displayColorIndex = (displayColorIndex + 1) % totalDisplayColors; 
    tft.fillScreen(displayColors[displayColorIndex]);          
    lastColorChange = currentMillis; 
    ringColorIndex = (ringColorIndex + 1) % totalRingColors; 
    delay(150); 
  } 
  
  lastTouchState = currentTouchState;

  // 5. SERVO CONTROL LOGIC (Triggers if someone is near OR sensor is touched)
  if (isSomeoneNear || currentTouchState == HIGH) {
    myServo.write(180); 
  } else {
    myServo.write(0);   
  }

  // 6. FALLBACK SCREEN AUTO-CYCLE (Every 10 seconds if no touch activity)
  if (currentMillis - lastColorChange >= colorInterval) {
    displayColorIndex = (displayColorIndex + 1) % totalDisplayColors;
    tft.fillScreen(displayColors[displayColorIndex]); 
    lastColorChange = currentMillis;
  }

  // 7. Apply Current Ring Light Color across all 8 LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    ring.setPixelColor(i, ringColors[ringColorIndex]); 
  }
  ring.show(); 

  // 8. Update Display Text Layout
  tft.setTextSize(2);
  if (displayColors[displayColorIndex] == ST7735_BLUE || displayColors[displayColorIndex] == ST7735_BLACK) {
    tft.setTextColor(ST7735_WHITE, displayColors[displayColorIndex]); 
  } else {
    tft.setTextColor(ST7735_YELLOW, displayColors[displayColorIndex]);
  }
  
  // Line 0: Title Header
  tft.setCursor(25, 10);
  tft.print("[BENTO BOX]");

  // Line 1: Temp
  tft.setCursor(12, 35);
  tft.print("Temp: ");
  if (!isnan(t)) { tft.print(t, 1); tft.print(" C "); } 
  else { tft.print("Err   "); }
  
  // Line 2: Hum
  tft.setCursor(12, 60);
  tft.print("Hum:  ");
  if (!isnan(h)) { tft.print(h, 1); tft.print(" % "); } 
  else { tft.print("Err   "); }

  // Line 3: Gas (Shifted up since Dist is gone)
  tft.setCursor(12, 85);
  tft.print("Gas:  ");
  tft.print(gasRaw);
  tft.print("    "); 

  // Line 4: Background Name
  tft.setCursor(12, 105);
  tft.print("BG:   ");
  tft.print(colorNames[displayColorIndex]);
  tft.print("     "); 

  // New Custom Footer status indicator (Shifted up to y=122 for clear visibility)
  tft.setTextSize(1);
  tft.setCursor(12, 122);
  if (isSomeoneNear || currentTouchState == HIGH) {
    tft.print("SERVO ACTIVE!      ");
  } else {
    tft.print("MAKE BREAK CREATE   ");
  }

  delay(50); 
}