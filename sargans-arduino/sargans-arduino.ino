#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <TM1637Display.h>
#include <Adafruit_NeoPixel.h>
#include <InputDebounce.h>
#include <Rotary.h>
#include <DS3232RTC.h>
#include <SoftwareSerial.h>
#include <DFMiniMp3.h>

// bundled libraries
#define RDM6300_SOFTWARE_SERIAL
#include "rdm6300.h"

#include "destinations.h"

// debouncing delay for buttons in ms
const int button_debounce_delay = 20;
const int strip_blink_period = 10;
const int strip_blink_count = 10;

// led settings
#define N_LED_BLOCKS 6
#define SCROLL_DELAY 8
#define CHAR_SPACING 1

// max intensity of neopixel (signal)
const int neo_intensity = 20;

// RTC CLOCK: I2C
// LED DISPLAY: SPI (+CS)
const int pin_led_cs = 10;
// OTHER PINS
const int pin_strip_switch = 2;
const int pin_strip = 3; // pwm needed
const int pin_rdm6300 = 4;
const int pin_mp3_rx = 9;
const int pin_mp3_tx = 8;
const int pin_neopixel = 14;
const int pin_rotary_switch = 17;
const int pin_rotary_a = 15;
const int pin_rotary_b = 16;
const int pin_clockdisplay_clk = 20;
const int pin_clockdisplay_data = 21;


// RFID reader
Rdm6300 rdm6300;
// LED banner -- unfortunately, PAROLA cannot be used (insane mmry requirements)
MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::FC16_HW, pin_led_cs, N_LED_BLOCKS);
// neopixel
Adafruit_NeoPixel pixels(1, pin_neopixel, NEO_GRB + NEO_KHZ800);
// 4-digit clock display
TM1637Display clock_display(pin_clockdisplay_clk, pin_clockdisplay_data);
// RTC
DS3232RTC rtc;

// rotary
Rotary rotary = Rotary(pin_rotary_a, pin_rotary_b);
InputDebounce rotary_button;
InputDebounce strip_button;

const int n_wagons = 48;
const uint32_t wagon_uids[n_wagons] = {
  361829,
  337311,
  343835,
  228613,
  228912,
  235002,
  250535,
  345008,
  252786,
  365067,

  355662,
  344216,
  344639,
  341953,
  339081,
  364832,
  258877,
  322125,
  203792,
  338900,

  566517,
  348127,
  329005,
  451390,
  290408,
  267758,
  186263,
  473183,
  911862,
  417940,
  
  477636,
  279181,
  // bacha prohazuju spatne nalepeny destinace 33-34, nutno otestovat
  285480, //451356,
  451356, //285480,
  293895,
  248222,
  486233,
  187191,
  342310,
  359534,
  
  273863,
  320530,
  504735,
  562113,
  538614,
  553631,
  413229,
  273606
};

#define BUF_SIZE  75
char message[BUF_SIZE] = "Sargans";

int playing_track = 0;

class Mp3Notify; 
typedef DFMiniMp3<HardwareSerial, Mp3Notify> DfMp3; 

// MP3 player notifier
class Mp3Notify {
public:
  static void OnError(DfMp3& mp3,uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("MP3 com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished(DfMp3& mp3, DfMp3_PlaySources source, uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(track);
    playing_track = 0;
  }
  static void OnPlaySourceOnline(DfMp3& mp3,DfMp3_PlaySources source) { }
  static void OnPlaySourceInserted(DfMp3& mp3,DfMp3_PlaySources source) { }
  static void OnPlaySourceRemoved(DfMp3& mp3,DfMp3_PlaySources source) { }
};

DfMp3 mp3(Serial1);

// volume range is (0-30)
const int mp3_volume = 26;

int previous_wagon = -1;

const int signal_mode_count = 4;
const int signal_mode_multiplier = 6;
enum signal_mode {SIG_GREEN, SIG_RED, SIG_ORANGE_BLINK, SIG_BLUE_BLINK};
signal_mode signal = SIG_RED;
bool signal_enabled = false;

bool strip_enabled = false;

int strip_blink_countdown = 0;

void setup()
{
  Serial.begin(115200);
  delay(2000); // just a small delay for the Serial

  Serial.println(F("Hello from Sargans!"));

  // --- init neopixel (traffic signal)
  Serial.println(F("Init neopixel..."));
  pixels.begin();
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(0, neo_intensity, 0));
  pixels.show();
  delay(500); // just a small delay without any meaning

  // --- init pins not handled separately
  pinMode(pin_strip, OUTPUT);
  pinMode(pin_rdm6300, INPUT_PULLUP);

  // --- init RFID
  Serial.println(F("Init RFID..."));
  rdm6300.begin(pin_rdm6300);
  delay(500); // just a small delay without any meaning

  // change init signal to blue
  pixels.setPixelColor(0, pixels.Color(0, 0, neo_intensity));
  pixels.show();

  // --- init LED banner
  Serial.println(F("Init banner..."));
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 1); // Use a value between 0 and 15 for brightness
  printText(0, N_LED_BLOCKS-1, message);
  mx.setShiftDataInCallback(scrollDataSource);
  mx.setShiftDataOutCallback(scrollDataSink);
  delay(500); // just a small delay without any meaning

  // setScrollMessage(message);

  // --- init clock display
  Serial.println(F("Init clock..."));
  clock_display.setBrightness(0x02);
  uint8_t data[4];
  data[0] = clock_display.encodeDigit(9);
  data[1] = clock_display.encodeDigit(3);
  data[2] = clock_display.encodeDigit(0);
  data[3] = clock_display.encodeDigit(2);
  clock_display.setSegments(data);
  delay(500); // just a small delay without any meaning

  // --- init RTC
  Serial.println(F("Init RTC..."));
  rtc.begin();

  // --- set RTC time
  // this is a super-not-nice way to handle this. just set it and uncomment here.
  // might not be needed anymore!
  // Serial.println(F("Set RTC to fixed value..."));
  // setTime( 13, 51,  0, 29, 11, 2022);   // H M S , D M Y
  // rtc.set(now());
  Serial.println(F("Get RTC time..."));
  tmElements_t tm;
  rtc.read(tm);
  Serial.print(tm.Hour);
  Serial.print(F(":"));
  Serial.print(tm.Minute);
  Serial.print(F(":"));
  Serial.println(tm.Second);
  
  // --- init MP3
  Serial.println(F("Init MP3..."));
  delay(500); // just a small delay without any meaning

  mp3.begin();

  uint16_t volume = mp3.getVolume();
  Serial.print(F("original volume: "));
  Serial.println(volume);
  mp3.setVolume(mp3_volume);
  volume = mp3.getVolume();
  Serial.print(F("new volume: "));
  Serial.println(volume);
  if (volume == 0) {
    mp3.begin();
  }

  uint16_t count = mp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  Serial.print(F("number of mp3 files: "));
  Serial.println(count);

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
  delay(500); // just a small delay without any meaning

  Serial.println(F("Init done."));
}

void loop() {
  unsigned long now = millis();
  bool blink = true;

  // --- process the LED banner display
  scrollText();

  // --- process the rotary encoder
  rotary.loop();

  // --- process the buttons
  rotary_button.process(now);
  strip_button.process(now);

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

      mp3.playMp3FolderTrack(current_wagon+1);
    }
  }

  // --- show the clock + init the blink flag
  tmElements_t tm;
  rtc.read(tm);
  blink = (tm.Second % 2 > 0);
  clock_display.showNumberDecEx(tm.Hour * 100 + tm.Minute, blink ? 0b01000000 : 0, true);
  
  // --- show the signals
  if (!signal_enabled) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  } else
  if (signal == SIG_RED) {
    pixels.setPixelColor(0, pixels.Color(neo_intensity, 0, 0));
  } else
  if (signal == SIG_GREEN) {
    pixels.setPixelColor(0, pixels.Color(0, neo_intensity, 0));
  }
  if (signal == SIG_BLUE_BLINK) {
    if (blink)
      pixels.setPixelColor(0, pixels.Color(0, 0, neo_intensity));
    else
      pixels.setPixelColor(0, pixels.Color(neo_intensity, neo_intensity, neo_intensity));
  }
  if (signal == SIG_ORANGE_BLINK) {
    if (blink)
      pixels.setPixelColor(0, pixels.Color(neo_intensity, neo_intensity, 0));
    else
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));
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

  // --- poll MP3
  mp3.loop();

  delay(10);
}

void rotary_button_pressedCallback(uint8_t pinIn) {
  Serial.print(F("Rotary switch (signal) pressed -> "));
  signal_enabled = !signal_enabled;
  Serial.println(signal_enabled);
}

void strip_button_pressedCallback(uint8_t pinIn) {
  Serial.print(F("Yellow switch (strip) pressed -> "));
  strip_enabled = !strip_enabled;
  Serial.println(strip_enabled);
}

// on rotary change
void rotate(Rotary& r) {
  int rpos = r.getPosition();
  signal = (rpos / 2) % signal_mode_count;
  Serial.print(F("Rotated the encoder to pos = "));
  Serial.print(rpos);
  Serial.print(F(" / signal = "));
  Serial.println(signal);
}
