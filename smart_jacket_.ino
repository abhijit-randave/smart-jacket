#define BLYNK_TEMPLATE_ID "TMPL3S16fwXGS"
#define BLYNK_TEMPLATE_NAME "Smart jacket"
#define BLYNK_AUTH_TOKEN "akPDbnW66T8jAAtU5Vis05uXlJ9sYsdh"

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <math.h> // For sqrt()
#include <MAX30100_PulseOximeter.h> // Include the MAX30100 library

Adafruit_MPU6050 mpu;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the I2C address (0x27) if necessary
MAX30100_PulseOximeter pox; // Initialize the MAX30100 sensor

// Wi-Fi credentials
char ssid[] = "Abhi";
char pass[] = "12345678";

// Threshold for detecting a fall (in m/s^2)
const float FALL_THRESHOLD = 12.0;

// Heart rate and SpO2 variables
float heartRate = 0;
float spo2 = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // Pause Zero, Leonardo, etc., until serial console opens

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  Serial.println("Adafruit MPU6050 with Blynk and MAX30100!");

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Try to initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    lcd.setCursor(0, 0);
    lcd.print("MPU6050 Error");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // Initialize the MAX30100 sensor
  if (!pox.begin()) {
    Serial.println("Failed to initialize MAX30100 sensor");
    lcd.setCursor(0, 0);
    lcd.print("MAX30100 Error");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MAX30100 Initialized!");
  
  lcd.setCursor(0, 0);
  lcd.print("MPU6050 & MAX30100");
  delay(2000);
}

void loop() {
  Blynk.run();

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Calculate the magnitude of acceleration
  float accelerationMagnitude = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  /* Read Heart Rate and SpO2 values from MAX30100 */
  pox.update(); // Update the MAX30100 sensor readings
  heartRate = pox.getHeartRate();
  spo2 = pox.getSpO2();

  /* Print out the values to Serial Monitor */
  Serial.print("X: ");
  Serial.print(a.acceleration.x);
  Serial.print(" Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(" Temp: ");
  Serial.print(temp.temperature);
  Serial.print(" AccMag: ");
  Serial.print(accelerationMagnitude);
  Serial.print(" HR: ");
  Serial.print(heartRate);
  Serial.print(" SpO2: ");
  Serial.println(spo2);

  /* Send data to Blynk */
  Blynk.virtualWrite(V0, temp.temperature);         // Temperature
  Blynk.virtualWrite(V1, a.acceleration.x);         // X-axis acceleration
  Blynk.virtualWrite(V2, a.acceleration.y);         // Y-axis acceleration
  Blynk.virtualWrite(V3, heartRate);                // Heart Rate
  Blynk.virtualWrite(V4, spo2);                     // SpO2

  /* Display on LCD */
  lcd.clear();
  if (accelerationMagnitude > FALL_THRESHOLD) {
    // Fall detected
    Serial.println("Fall Detected!");
    lcd.setCursor(0, 0);
    lcd.print("Fall Detected!");
    lcd.setCursor(0, 1);
    lcd.print("AccMag: ");
    lcd.print(accelerationMagnitude, 1);
    Blynk.virtualWrite(V6, "Fall Detected!"); // Send fall detection alert
  } else {
    // Normal operation: Show sensor values
    lcd.setCursor(0, 0);
    lcd.print("X:");
    lcd.print(a.acceleration.x, 1);
    lcd.print(" Y:");
    lcd.print(a.acceleration.y, 1);
    lcd.setCursor(0, 1);
    lcd.print("HR:");
    lcd.print(heartRate, 1);
    lcd.print("bpm");
    lcd.setCursor(0, 1);
    lcd.print("SpO2:");
    lcd.print(spo2, 1);
    lcd.print("%");
    Blynk.virtualWrite(V6, "No Fall"); // Normal status
  }

  delay(500);
}
