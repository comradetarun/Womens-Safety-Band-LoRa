#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// GPIO Pins
#define BUTTON_PIN   15
#define LED_PIN       2
#define BUZZER_PIN    4

// LoRa pins
#define LORA_SCK     18
#define LORA_MISO    19
#define LORA_MOSI    23
#define LORA_NSS      5
#define LORA_RST     14
#define LORA_DIO0    26

// GPS Serial
#define GPS_RX       16   // ESP32 RX2 ← GPS TX
#define GPS_TX       17   // ESP32 TX2 → GPS RX

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

bool buttonPressed = false;
unsigned long lastDebounce = 0;
#define DEBOUNCE_MS 200

void blinkLED() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(300);
  }
}

void beepBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
}

void sendAlert(float lat, float lng) {
  String msg = "SOS," + String(lat, 6) + "," + String(lng, 6);
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();
  Serial.println("Sent: " + msg);
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Active LOW
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // LoRa init
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {   // Change to 868E6 or 915E6 if needed
    Serial.println("LoRa init failed!");
    while (1);
  }
  Serial.println("Safety band ready.");
}

void loop() {
  // Feed GPS
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  // Button check (active LOW with INPUT_PULLUP)
  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long now = millis();
    if (now - lastDebounce > DEBOUNCE_MS) {
      lastDebounce = now;

      Serial.println("Button pressed! Sending SOS...");
      blinkLED();    // LED blinks 3 times
      beepBuzzer();  // Buzzer sounds once

      float lat = gps.location.isValid() ? gps.location.lat() : 0.0;
      float lng = gps.location.isValid() ? gps.location.lng() : 0.0;
      sendAlert(lat, lng);
    }
  }
}
