#include "FastLED.h"
#include <RTCZero.h>

// How many leds in your strip?
const int pixelCount = 60;


// Define the array of leds
CRGB leds[pixelCount];
unsigned long color = 0x0F0F0F;

RTCZero rtc;
int lastSecond = 0;

void setup() {

  FastLED.addLeds<APA102, 8, 9>(leds, pixelCount);
  Serial.begin(9600);
  while (!Serial);
  String compileTime = String(__TIME__);
  String token = compileTime.substring(0, 2);
  int h = token.toInt();

  token = compileTime.substring(3, 5);
  int m = token.toInt();
  token = compileTime.substring(6, 8);
  int s = token.toInt();

  pinMode(LED_BUILTIN, OUTPUT);     // set builtin LED to output  cardPresent = initializeCard();   // check to see if SD card is accessible

  rtc.begin();                      // start the realtime clock
  rtc.setTime(h, m, s);
  rtc.setAlarmTime(15, 13, 00);     // set an alarm for processor wakeup
  rtc.enableAlarm(rtc.MATCH_SS);    // enable it
  rtc.attachInterrupt(alarmMatch);  // have the wakeup happen when the RTC seconds matches 00

  // to set the date and time (epoch, or seconds since 1/1/1970) from a POSIX machine, type:
  // $ date +T%s
  // thanks to Paul Stoffregen for this trick


}

void loop() {
  int toDim = -1;
  if (rtc.getSeconds() != lastSecond) {
    Serial.println(getTimeStamp());
    // loop over all the pixels:
    for (int pixel = 0; pixel < pixelCount; pixel++) {
      if ((60 - pixel) > rtc.getSeconds())  {
        leds[pixel] = color;   // turn on the pixel
      } else {
        leds[pixel]--;
      }
    }
    //    int h = rtc.getHours() % 12 * 5;
    //
    //    for (int x = h; x < h + 5; x++) {
    //      leds[x] = 0xFFFFFF;
    //    }
    int m = 60 - rtc.getMinutes();
    leds[m] = 0xFFFFFF;
    lastSecond = rtc.getSeconds();
  }

  if (Serial.available() > 10) {
    readTime();
  }

  FastLED.show();             // update the pixels
}


void alarmMatch() {
  Serial.println("ping");
}

void readTime() {
  digitalWrite(LED_BUILTIN, HIGH);
  // if you get the T:

  if (Serial.findUntil("T", "\n")) {
    unsigned long epoch = Serial.parseInt();  // set the epoch
    rtc.setEpoch(epoch);
    Serial.println(getTimeStamp());   // print the timestamp
  }
  // serial delay over, turn off the LED:
  digitalWrite(LED_BUILTIN, LOW);
}

String getTimeStamp() {
  // make an ISO8601 formatted timestring:
  //  e.g. 2018 - 02 - 21T01: 49: 51.296Z
  String timestamp = "";
  if (rtc.getHours() <= 9) timestamp += "0";
  timestamp += rtc.getHours();
  timestamp += ":";
  if (rtc.getMinutes() <= 9) timestamp += "0";
  timestamp += rtc.getMinutes();
  timestamp += ":";
  if (rtc.getSeconds() <= 9) timestamp += "0";
  timestamp += rtc.getSeconds();
  return timestamp;
}
