// system libraries
#include <Arduino.h>

// bundled libraries
#define RDM6300_SOFTWARE_SERIAL
#include "rdm6300.h"

const int pin_rdm6300 = 4;

// RFID reader
Rdm6300 rdm6300;

void setup()
{
  Serial.begin(115200);
  delay(2000); // just a small delay

  Serial.println(F("Hello from Sargans!"));

  // --- init pins not handled separately
  pinMode(pin_rdm6300, INPUT_PULLUP);

  // --- init RFID
  Serial.println(F("Init RFID..."));
  rdm6300.begin(pin_rdm6300);

  Serial.println(F("Init done."));
}

void loop() {
  unsigned long now = millis();

  // --- process RFID
  if (rdm6300.get_new_tag_id()) {
    uint32_t uid = rdm6300.get_tag_id();
    Serial.print(F("Read RFID "));
    Serial.print(uid, HEX);
    Serial.print(F(" = "));
    Serial.println(uid);
  }

  delay(10);
}
