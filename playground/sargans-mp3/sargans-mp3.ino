#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFMiniMp3.h>

const int pin_mp3_rx = 9;
const int pin_mp3_tx = 8;

int playing_track = 0;
int target_track = 0;

// MP3 player notifier
class Mp3Notify {
public:
  static void OnError(uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("MP3 com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished(DfMp3_PlaySources source, uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(track);
    playing_track = 0;
  }
  static void OnPlaySourceOnline(DfMp3_PlaySources source) { }
  static void OnPlaySourceInserted(DfMp3_PlaySources source) { }
  static void OnPlaySourceRemoved(DfMp3_PlaySources source) { }
};

DFMiniMp3<HardwareSerial, Mp3Notify> mp3(Serial1);

const int mp3_volume = 28;

void setup()
{
  Serial.begin(115200);
  delay(2000); // just a small delay

  mp3.begin();

  uint16_t volume = mp3.getVolume();
  Serial.print(F("original volume: "));
  Serial.println(volume);
  mp3.setVolume(mp3_volume);
//  while (volume != mp3_volume) {
    volume = mp3.getVolume();
    Serial.print(F("new volume: "));
    Serial.println(volume);
    if (volume == 0) {
      mp3.begin();
    }
//    if (volume != mp3_volume) {
//      delay(500);
//      mp3.setVolume(mp3_volume);
//    }
//  }

  uint16_t count = mp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  Serial.print(F("number of mp3 files: "));
  Serial.println(count);

  Serial.println(F("Init done."));

  playing_track = 1;
  target_track = 1;
  Serial.print(F("playing track "));
  Serial.println(target_track);
  mp3.playMp3FolderTrack(target_track);

}

void loop() {

  // --- poll MP3
  mp3.loop();

  if (playing_track == 0) {
      target_track++;
  Serial.print(F("playing track "));
  Serial.println(target_track);
      mp3.playMp3FolderTrack(target_track);
      playing_track = target_track;
  }

  delay(10);
}
