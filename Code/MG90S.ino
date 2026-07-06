// Define the GPIO pin connected to the orange signal wire
const int servoPin = 13;

// Servo timing configuration (Standard for MG90s)
const int pwmChannel = 0;
const int freq = 50;           // 50Hz frequency
const int resolution = 16;     // 16-bit resolution (0 to 65535)

// Convert degrees (0-180) to 16-bit PWM duty cycle
int angleToDuty(int angle) {
  // MG90s usually takes ~500us to ~2400us pulse widths
  // For 50Hz (20000us period) at 16-bit resolution:
  long minDuty = (500 * 65535) / 20000;
  long maxDuty = (2400 * 65535) / 20000;
  return minDuty + ((maxDuty - minDuty) * angle / 180);
}

void setup() {
  Serial.begin(115200);

  // Set up the built-in ESP32 PWM features
  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    // For ESP32 Board Core 3.x (Your current version)
    ledcAttach(servoPin, freq, resolution);
  #else
    // For older ESP32 Board Cores
    ledcSetup(pwmChannel, freq, resolution);
    ledcAttachPin(servoPin, pwmChannel);
  #endif
}

void loop() {
  // Sweep from 0 to 180 degrees
  for (int angle = 0; angle <= 180; angle += 2) {
    int duty = angleToDuty(angle);
    #if ESP_ARDUINO_VERSION_MAJOR >= 3
      ledcWrite(servoPin, duty);
    #else
      ledcWrite(pwmChannel, duty);
    #endif
    delay(15); 
  }

  delay(500);

  // Sweep back from 180 to 0 degrees
  for (int angle = 180; angle >= 0; angle -= 2) {
    int duty = angleToDuty(angle);
    #if ESP_ARDUINO_VERSION_MAJOR >= 3
      ledcWrite(servoPin, duty);
    #else
      ledcWrite(pwmChannel, duty);
    #endif
    delay(15);
  }

  delay(500);
}