#define BLYNK_TEMPLATE_ID "TMPL35igXYI13"
#define BLYNK_TEMPLATE_NAME "Heart Rate Monitor"
#define BLYNK_AUTH_TOKEN "JJonuu5fpmPmAxWmVYuMag9-WccFOPl7"

#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Ticker.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin Definitions
#define RELAY 19          // Pin for Relay (Simulating a fan)
#define PULSE_PIN 35      // Pin for Pulse Sensor (ESP32 GPIO 35)
#define ALERT_LED 32      // Pin for Alert LED

// Blynk Authentication Token
char auth[] = BLYNK_AUTH_TOKEN;
const char* ssid = "";
const char* password = "";

// Heart Rate Thresholds
int minHeartRate = 50;
int maxHeartRate = 120;

// Timer for regular pulse checks
Ticker ticker;

void sendHeartRateToBlynk(int heartRate) {
  Blynk.virtualWrite(V0, heartRate);  // Sending heart rate data to Virtual Pin V0
}

void checkHeartRate() {
  // Read pulse sensor value
  int pulseValue = analogRead(PULSE_PIN);
  
  // Convert the sensor value to voltage and heart rate
  float voltage = pulseValue * (3.3 / 4095.0);
  int heartRate = (voltage / 3.3) * 675;  // Calculate heart rate (approximation)

  // Send heart rate to Blynk
  sendHeartRateToBlynk(heartRate);

  // Check heart rate and update display and controls accordingly
  if (heartRate < minHeartRate) {
    // Display message on OLED for low heart rate
    display.clearDisplay();
    display.setTextSize(1);      
    display.setTextColor(SSD1306_WHITE);  
    display.setCursor(0, 0);
    display.print("Heart Rate is below");
    display.setCursor(0, 15);
    display.print("Minimum Threshold");
    display.setCursor(20, 30);
    display.print(String(heartRate) + " bpm");
    display.display();
    
    digitalWrite(RELAY, LOW);      // Turn off the relay
    digitalWrite(ALERT_LED, LOW);   // Turn off the alert LED
  } 
  else if (heartRate > maxHeartRate) {
    // Display message on OLED for high heart rate
    display.clearDisplay();
    display.setTextSize(1);      
    display.setTextColor(SSD1306_WHITE);  
    display.setCursor(0, 0);
    display.print("Heart Rate is above");
    display.setCursor(0, 15);
    display.print("Maximum Threshold");
    display.setCursor(20, 30);
    display.print(String(heartRate) + " bpm");
    display.setCursor(10, 50);
    display.print("Speed set at Max");
    display.display();

    digitalWrite(RELAY, HIGH);     // Turn on the relay (simulating a fan at high speed)
    digitalWrite(ALERT_LED, HIGH);  // Turn on the alert LED
  } 
  else {
    // Display message for normal heart rate
    display.clearDisplay();
    display.setTextSize(1);      
    display.setTextColor(SSD1306_WHITE);  
    display.setCursor(0, 0);
    display.print("Heart Rate is Normal");
    display.setCursor(20, 40);
    display.print(String(heartRate) + " bpm");
    display.display();

    digitalWrite(RELAY, LOW);      // Turn off the relay
    digitalWrite(ALERT_LED, LOW);   // Turn off the alert LED
  }

  Serial.println("Heart Rate: " + String(heartRate) + " bpm");
}

void setup() {
  Serial.begin(115200);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 30);
  display.println("Heart Rate Monitor");
  display.display();

  // Initialize pins
  pinMode(RELAY, OUTPUT);
  pinMode(ALERT_LED, OUTPUT);
  pinMode(PULSE_PIN, INPUT);

  // Initialize Blynk
  Blynk.begin(auth, ssid, password);

  // Set up a ticker to call checkHeartRate every 2 seconds
  ticker.attach(2.0, checkHeartRate); // Call every 2 seconds
}

void loop() {
  Blynk.run();  // Keep Blynk connection alive
}
