#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <ESP32Servo.h>

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DHT22
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// HC-SR04
#define TRIG_PIN 5
#define ECHO_PIN 18

// MQ2
#define MQ2_PIN 34
#define GAS_THRESHOLD 2500

// LEDs
#define RED_LED 26
#define GREEN_LED 27

// Buzzer
#define BUZZER 15

// Push Button
#define BUTTON_PIN 13

// Servo
#define SERVO_PIN 19
Servo barrierServo;

// Relay
#define RELAY_PIN 23

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) return 999;

  return duration * 0.034 / 2.0;
}

void buzzerOn() {
  tone(BUZZER, 1000);
}

void buzzerOff() {
  noTone(BUZZER);
}

void setup() {

  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(RELAY_PIN, OUTPUT);

  dht.begin();

  barrierServo.attach(SERVO_PIN);
  barrierServo.write(0);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Failed");
    while (1);
  }

  display.clearDisplay();
  display.display();
}

void loop() {

  float distance = getDistance();
  int gasValue = analogRead(MQ2_PIN);
  float temperature = dht.readTemperature();

  bool parkingFull = distance < 10;
  bool pollutionAlert = gasValue > GAS_THRESHOLD;
  bool fireAlert = temperature > 35;
  bool emergencyButton = digitalRead(BUTTON_PIN) == LOW;
  bool cityEmergency = pollutionAlert && fireAlert;

  // -------------------------
  // PARKING LEDs
  // -------------------------
 // LEDs

if (cityEmergency) {

  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);

}
else if (parkingFull) {

  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);

}
else {

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

}

  // -------------------------
  // RELAY
  // -------------------------
  digitalWrite(RELAY_PIN, fireAlert ? HIGH : LOW);

  // -------------------------
  // SERVO
  // -------------------------
  if (emergencyButton || cityEmergency) {
    barrierServo.write(90);
  } else {
    barrierServo.write(0);
  }

  // -------------------------
  // BUZZER
  // -------------------------
  if (pollutionAlert || fireAlert || emergencyButton) {
    buzzerOn();
  } else {
    buzzerOff();
  }

  // -------------------------
  // OLED DISPLAY
  // -------------------------
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (cityEmergency) {

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("CITY");

    display.println("EMERGENCY");

    display.setTextSize(1);
    display.println("EVACUATE AREA");
  }
  else {

    display.setCursor(0, 0);

    display.print("Parking: ");
    display.println(parkingFull ? "FULL" : "AVAILABLE");

    display.print("Gas: ");
    display.println(pollutionAlert ? "ALERT" : "SAFE");

    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");

    display.print("Barrier: ");
    display.println((emergencyButton || cityEmergency) ? "OPEN" : "CLOSED");

    display.print("Status: ");

    if (emergencyButton) {
      display.println("EMERGENCY");
    }
    else if (pollutionAlert || fireAlert) {
      display.println("WARNING");
    }
    else {
      display.println("NORMAL");
    }
  }

  display.display();

  // Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);

  Serial.print(" | Gas: ");
  Serial.print(gasValue);

  Serial.print(" | Temp: ");
  Serial.println(temperature);

  delay(500);
}