// system libraries
#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <TM1637Display.h>
#include <Adafruit_NeoPixel.h>
#include <InputDebounce.h>
#include <Rotary.h>
#include <DS3232RTC.h>

// bundled libraries
#define RDM6300_SOFTWARE_SERIAL
#include "rdm6300.h"

// data
#include "destinations.h"

// debouncing delay for buttons in ms
const int button_debounce_delay = 20; 
const int strip_blink_period = 10;
const int strip_blink_count = 10;

// led settings
#define N_LED_BLOCKS 6
#define SCROLL_DELAY 4
#define CHAR_SPACING 1

// max intensity of neopixel (signals)
const int neo_intensity = 20;

// RTC CLOCK: I2C
// LED DISPLAY: SPI (+CS)
const int pin_led_cs = 10;
// OTHER PINS
const int pin_strip_switch = 2;
const int pin_strip = 3; // pwm needed
const int pin_rdm6300 = 4;
const int pin_speaker = 5; // pwm needed
const int pin_signal1_switch = 7;
const int pin_neopixel = 14;
const int pin_rotary_switch = 17;
const int pin_rotary_a = 15;
const int pin_rotary_b = 16;
const int pin_clockdisplay_clk = 20;
const int pin_clockdisplay_data = 21;
const int pin_mp3_rx = 9;
const int pin_mp3_tx = 8;  

// RFID reader
Rdm6300 rdm6300;
// LED banner -- unfortunately, PAROLA cannot be used (insane mmry requirements)
MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::FC16_HW, pin_led_cs, N_LED_BLOCKS);
// neopixels 
Adafruit_NeoPixel pixels(2, pin_neopixel, NEO_GRB + NEO_KHZ800);
// 4-digit clock display
TM1637Display clock_display(pin_clockdisplay_clk, pin_clockdisplay_data);
// RTC
DS3232RTC rtc;

// rotary
Rotary rotary = Rotary(pin_rotary_a, pin_rotary_b);
InputDebounce rotary_button;
InputDebounce strip_button;
InputDebounce signal1_button;

const int n_wagons = 4;
const uint32_t wagon_uids[n_wagons] = {
  273606,
  337311,
  361829,
  186263
};

#define BUF_SIZE  75
char message[BUF_SIZE] = "Sargans SBB";

int previous_wagon = -1;

enum signal1_mode {S1_GREEN, S1_RED};
signal1_mode signal1 = S1_RED;

const int signal2_mode_count = 4;
const int signal2_mode_multiplier = 6;
enum signal2_mode {S2_GREEN, S2_RED, S2_ORANGE_BLINK, S2_BLUE_BLINK};
signal2_mode signal2 = S2_RED;
bool signal2_enabled = true;

bool strip_enabled = false;

int strip_blink_countdown = 0;

void setup()
{
  Serial.begin(115200);
  delay(2000); // just a small delay

  Serial.println(F("Hello from Sargans!"));

  // --- init pins not handled separately
  pinMode(pin_strip, OUTPUT);
  pinMode(pin_rdm6300, INPUT_PULLUP);

  // --- init RFID
  Serial.println(F("Init RFID..."));
  rdm6300.begin(pin_rdm6300);

  // --- init LED banner
  Serial.println(F("Init banner..."));
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 1); // Use a value between 0 and 15 for brightness
  printText(0, N_LED_BLOCKS-1, message);
  mx.setShiftDataInCallback(scrollDataSource);
  mx.setShiftDataOutCallback(scrollDataSink);

  // setScrollMessage(message);
  
  // --- init clock display
  Serial.println(F("Init clock..."));
  clock_display.setBrightness(0x02);
  uint8_t data[4];
  data[0] = clock_display.encodeDigit(random(10));
  data[1] = clock_display.encodeDigit(random(10));
  data[2] = clock_display.encodeDigit(random(10));
  data[3] = clock_display.encodeDigit(random(10));
  clock_display.setSegments(data);

  // --- init RTC
  rtc.begin();

  // --- set RTC time
  // this is a super-not-nice way to handle this. just set it and uncomment here.
  // might not be needed anymore!    
  //setTime( 11, 20,  0, 24, 4, 2022);   // H M S , D M Y
  //rtc.set(now());
  
  // --- init neopixels (traffic signals)
  Serial.println(F("Init neopixels..."));
  pixels.begin();
  pixels.clear();
  pixels.show();

  // --- init rotary switch
  Serial.println(F("Init rotary..."));
  pinMode(pin_rotary_a, INPUT);
  pinMode(pin_rotary_b, INPUT);
  rotary.setChangedHandler(rotate);
  
  // --- init the buttons
  rotary_button.registerCallbacks(rotary_button_pressedCallback, NULL, NULL, NULL);
  rotary_button.setup(pin_rotary_switch, button_debounce_delay, InputDebounce::PIM_INT_PULL_UP_RES);
  strip_button.registerCallbacks(strip_button_pressedCallback, NULL, NULL, NULL);
  strip_button.setup(pin_strip_switch, button_debounce_delay, InputDebounce::PIM_INT_PULL_UP_RES);
  signal1_button.registerCallbacks(signal1_button_pressedCallback, NULL, NULL, NULL);
  signal1_button.setup(pin_signal1_switch, button_debounce_delay, InputDebounce::PIM_INT_PULL_UP_RES);

  Serial.println(F("Init done."));
}

void loop() {
  unsigned long now = millis();

  // --- process the LED banner display
  scrollText();

  // --- process the rotary encoder
  rotary.loop();

  // --- process the buttons
  rotary_button.process(now);
  strip_button.process(now);
  signal1_button.process(now);

  // --- process RFID
  if (rdm6300.get_new_tag_id()) {
    uint32_t uid = rdm6300.get_tag_id();
    Serial.print(F("Read RFID "));
    Serial.print(uid, HEX);
    int current_wagon = -1;
    for (int i = 0; i < n_wagons; ++i) {
      if (wagon_uids[i] == uid) {
        current_wagon = i;
        break;
      }
    }
    Serial.print(F(" -> wagon "));
    Serial.println(current_wagon);
    if (current_wagon > -1 && current_wagon != previous_wagon) {
      Serial.print(F(" => printing message "));
      Serial.println(destinations[current_wagon]);
      setScrollMessage(destinations[current_wagon]);
      previous_wagon = current_wagon;
      strip_blink_countdown = strip_blink_period * strip_blink_count;
    }
  }

  // --- show the clock + init the blink flag
  tmElements_t tm;
  rtc.read(tm);
  bool blink = (tm.Second % 2 > 0);
  clock_display.showNumberDecEx(tm.Hour * 100 + tm.Minute, blink ? 0b01000000 : 0, true); 

  // --- show the signals
  if (signal1 == S1_RED) {
    pixels.setPixelColor(0, pixels.Color(neo_intensity, 0, 0));   
  } else
  if (signal1 == S1_GREEN) {
    pixels.setPixelColor(0, pixels.Color(0, neo_intensity, 0));   
  }
  if (!signal2_enabled) {
    pixels.setPixelColor(1, pixels.Color(0, 0, 0));
  } else
  if (signal2 == S2_RED) {
    pixels.setPixelColor(1, pixels.Color(neo_intensity, 0, 0));   
  } else
  if (signal2 == S2_GREEN) {
    pixels.setPixelColor(1, pixels.Color(0, neo_intensity, 0));   
  }
  if (signal2 == S2_BLUE_BLINK) {
    if (blink)
      pixels.setPixelColor(1, pixels.Color(0, 0, neo_intensity));   
    else
      pixels.setPixelColor(1, pixels.Color(neo_intensity, neo_intensity, neo_intensity));   
  }
  if (signal2 == S2_ORANGE_BLINK) {
    if (blink)
      pixels.setPixelColor(1, pixels.Color(neo_intensity, neo_intensity, 0));   
    else
      pixels.setPixelColor(1, pixels.Color(0, 0, 0));   
  }
  pixels.show();

  // --- handle the led strip
  if (strip_blink_countdown > 0) {
    strip_blink_countdown--;
    int m = strip_blink_countdown / strip_blink_period;
    if (m % 2 == 0)
      analogWrite(pin_strip, 250);
    else
      analogWrite(pin_strip, 0);  
  } else {
    if (strip_enabled)
      analogWrite(pin_strip, 50);
    else
      analogWrite(pin_strip, 0);  
  }

  delay(10);
}

void signal1_button_pressedCallback(uint8_t pinIn) {
  Serial.print(F("Red switch (signal1) pressed -> "));
  signal1 = (signal1 == S1_RED) ? S1_GREEN : S1_RED;
  Serial.println(signal2_enabled);
}

void rotary_button_pressedCallback(uint8_t pinIn) {
  Serial.print(F("Rotary switch (signal2) pressed -> "));
  signal2_enabled = !signal2_enabled;
  Serial.println(signal2_enabled);
}

void strip_button_pressedCallback(uint8_t pinIn) {
  Serial.print(F("Yellow switch (strip) pressed -> "));
  strip_enabled = !strip_enabled;
  Serial.println(strip_enabled);
}

// on rotary change
void rotate(Rotary& r) {
  int rpos = r.getPosition();
  signal2 = (rpos / 2) % signal2_mode_count;
  Serial.print(F("Rotated the encoder to pos = "));
  Serial.print(rpos);
  Serial.print(F(" / signal2 = "));
  Serial.println(signal2);
}
