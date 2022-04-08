#include <Arduino.h>

#include <MD_Parola.h>
#include <MD_MAX72xx.h>

#include "rdm6300.h"

const int pin_rdm6300 = 5;

const int pin_led_data = 2;
const int pin_led_clk = 4;
const int pin_led_cs = 3;

Rdm6300 rdm6300;
MD_Parola banner = MD_Parola(MD_MAX72XX::FC16_HW, pin_led_data, pin_led_clk, pin_led_cs, 4);

const int n_wagons = 4;
const uint32_t wagon_uids[n_wagons] = {
  273606,
  337311,
  361829,
  186263
};

char destinations[n_wagons][80] = {
  "S12 Bad Ragaz - Maienfeld - Chur",
  "S4 Sevelen - Buchs SG - Salez - Sennwald - Rorschach - St. Gallen",
  "S4 Mels - Flums - Walenstadt - Uznach",
  "IC3 Zuerich HB - Basel SBB"
};

int previous_wagon = -1;

void setup()
{
  Serial.begin(115200);
  delay(2000); // just a small delay

  Serial.println("Hello from Sargans!");

  Serial.println(F("Init RFID..."));
  rdm6300.begin(pin_rdm6300);

  Serial.println(F("Init banner..."));
  banner.begin();
  // the number is ms-delay for how long the message stays there
  banner.displayText("Sargans", PA_CENTER, banner.getSpeed(), 2000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  // intensity 1-15
  banner.setIntensity(2);
  // speed is delay-ish (0 = fastest, 20 = a bit choppy)
  banner.setSpeed(20);

  Serial.println(F("Init done."));
}

void loop()
{
  if (rdm6300.get_new_tag_id()) {
    uint32_t uid = rdm6300.get_tag_id();
    Serial.print(uid, HEX);
    int current_wagon = -1;
    for (int i = 0; i < n_wagons; ++i) {
      if (wagon_uids[i] == uid) {
        current_wagon = i;
        break;
      }
    }
    Serial.print(F(" -> "));
    Serial.println(current_wagon);
    if (current_wagon > -1 && current_wagon != previous_wagon) {
      Serial.println(destinations[current_wagon]);
      banner.displayText(destinations[current_wagon], PA_CENTER, banner.getSpeed(), 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      previous_wagon = current_wagon;
    }
  }

  if (banner.displayAnimate()) banner.displayClear();

  delay(10);
}
