#include <SPI.h>
#include <LoRa.h>

// LoRa pins (same wiring on receiver ESP32)
#define LORA_SCK     18
#define LORA_MISO    19
#define LORA_MOSI    23
#define LORA_NSS      5
#define LORA_RST     14
#define LORA_DIO0    26

// Alert LED on receiver
#define ALERT_LED    2

void blinkAlert() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(ALERT_LED, HIGH);
    delay(200);
    digitalWrite(ALERT_LED, LOW);
    delay(200);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ALERT_LED, OUTPUT);
  digitalWrite(ALERT_LED, LOW);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {   // Must match sender frequency
    Serial.println("LoRa init failed!");
    while (1);
  }
  Serial.println("Receiver ready, listening...");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }

    Serial.print("Received: ");
    Serial.println(received);
    Serial.print("RSSI: ");
    Serial.println(LoRa.packetRssi());

    // Parse SOS message
    if (received.startsWith("SOS,")) {
      Serial.println("*** SOS ALERT RECEIVED! ***");

      // Extract GPS coordinates
      int comma1 = received.indexOf(',');
      int comma2 = received.indexOf(',', comma1 + 1);
      String lat = received.substring(comma1 + 1, comma2);
      String lng = received.substring(comma2 + 1);

      Serial.println("Latitude:  " + lat);
      Serial.println("Longitude: " + lng);

      blinkAlert();  // Flash LED rapidly on receiver
    }
  }
}
