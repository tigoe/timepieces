#include <Encoder.h>

#include <RTCZero.h>

#include <Adafruit_DotStar.h>
#include <SPI.h>
#define DATAPIN    8
#define CLOCKPIN   9

// How many leds in your strip?
const int pixelCount = 60;

Adafruit_DotStar leds = Adafruit_DotStar( pixelCount, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
unsigned long color = 0x0F0F0F;

const int secondsButton = 2;
Encoder minutesKnob(4, 5);
int  lastmKnob = -1;

RTCZero rtc;
int lastMinute = 0;
int lastSecond = 0;
byte secondLevel = 0x0F;
byte minuteLevel = 0xFF;
byte secondFade = 0x0F;
byte minuteFade = 0xFF;

void setup() {
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
  pinMode(secondsButton, INPUT_PULLUP);
  rtc.begin();                      // start the realtime clock
  rtc.setTime(h, m, s);

  leds.begin();
  leds.show();
}

void loop() {

  if (digitalRead(secondsButton) == LOW) {
    rtc.setSeconds(0);
  }

  int mKnob = minutesKnob.read();
  if (mKnob != lastmKnob) {
    int m = rtc.getMinutes();
    if (mKnob > lastmKnob) {
      m++;
    } else {
      m--;
    }
    if (m > 59) m = m % 60;
    rtc.setMinutes(m);
    lastmKnob = mKnob;
  }

  // when the second changes, update lastSecond
  if (rtc.getSeconds() != lastSecond) {
    Serial.println(getTimeStamp());
    // set all LED less than the current second to a low level
    // loop over all the pixels:
    for (int pixel = 0; pixel < pixelCount; pixel++) {
      if ((60 - pixel) > rtc.getSeconds())  {
        changePixel(pixel, secondLevel);
      }
    }
    lastSecond = rtc.getSeconds();
    secondFade = secondLevel;
  }

  // when the minute changes, update lastMinute
  if (60 - rtc.getMinutes() != lastMinute) {
    // set the LED corresponding to the current minute to a high level
    int m = 60 - rtc.getMinutes();
    changePixel(m, minuteLevel);
    lastMinute = m;
    minuteFade = minuteLevel;
  }

  // might need to wrap these in a sub-second interval:
  // fade the LED corresponding to the previous second
  if (secondFade > 0) {
    changePixel(lastSecond, secondLevel);
    secondFade--;
  }
  // fade the LED corresponding to the previous minute
  if (minuteFade > 0) {
    changePixel(lastMinute, minuteLevel);
    minuteFade--;
  }

  if (Serial.available() > 10) {
    readTime();
  }

  leds.show();             // update the pixels
}

void changePixel(int thisPixel, byte thisLevel) {
  long level = thisLevel | (thisLevel << 8) | (thisLevel << 16);
  leds.setPixelColor(thisPixel, thisLevel);
}

void readTime() {
  // to set the date and time (epoch, or seconds since 1/1/1970) from a POSIX machine, type:
  // $ date +T%s
  // thanks to Paul Stoffregen for this trick

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
