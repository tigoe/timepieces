/*
    Version for the Trinket M0

*/
#include <RTCZero.h>
#include <Adafruit_DotStar.h>
#include <SPI.h>

#define DATAPIN    4
#define CLOCKPIN   3

const int secondsButton = 0;
const int minutesButton = 1;

// How many leds in your strip?
const int pixelCount = 60;

Adafruit_DotStar strip = Adafruit_DotStar( pixelCount, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
unsigned long secondColor = 0x0F0F0F;
unsigned long minuteColor = 0xFEFEFE;

RTCZero rtc;
int thisMinute, thisSecond;
volatile boolean minuteReset = false;
volatile int lastMinutePixel = 0;
int lastmButtonState = HIGH;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);     // set builtin LED to output
  pinMode(secondsButton, INPUT_PULLUP);
  pinMode(minutesButton, INPUT_PULLUP);
  rtc.begin();                      // start the realtime clock
  rtc.setTime(00, 50, 55);
  rtc.setAlarmTime(00, 00, 59);     // set an alarm for processor wakeup
  rtc.enableAlarm(rtc.MATCH_SS);    // enable it
  rtc.attachInterrupt(alarmMatch);  // have the wakeup happen when the RTC seconds matches 00

  strip.begin();
  strip.show();
  Serial.println(getTimeStamp());
}

void loop() {
  unsigned long thisColor = 0;
  //   if (thisSecond != rtc.getSeconds()) {
  //    Serial.println(getTimeStamp());
  //  }
  // get the time:
  thisMinute = rtc.getMinutes();
  thisSecond = rtc.getSeconds();

  // convert to LED positions:
  int secondPos = 59 - thisSecond;
  int minutePos = 59 - thisMinute;

  // read seconds reset button:
  if (digitalRead(secondsButton) == LOW) {
    rtc.setSeconds(0);
  }

  // read minutes update button:
  int mButtonState = digitalRead(minutesButton);
  if (mButtonState != lastmButtonState) {
    if (mButtonState == LOW) {
      int m = rtc.getMinutes() + 1; // add 1 to minutes
      m = m % 60;         // limit it to a range from 0 to 59
      rtc.setMinutes(m);
    }
    lastmButtonState = mButtonState;
  }

  // loop over all the pixels:
  for (int pixel = 0; pixel < pixelCount; pixel++) {
    unsigned long currentColor = strip.getPixelColor(pixel);
    // if it's <= the second
    if (pixel <= secondPos) {   // if it's the second or less,
      // set it to secondColor
      if (currentColor > secondColor) {
        thisColor = fadeColor(currentColor, -1);
      } else {
        thisColor = fadeColor(currentColor, 1);
      }
    } else {
      // if it's supposed to be off, fade it all the way:
      if (currentColor > 0) {
        thisColor = fadeColor(currentColor, -1);
      } else {
        thisColor = 0;
      }
    }

    // if it's the minute,
    if (pixel == minutePos) {   // if it's the minute,
      //    if it's < minuteColor
      if (currentColor < minuteColor) {
        thisColor = fadeColor(currentColor, 1);
      }
      if (currentColor > minuteColor) {
        thisColor = fadeColor(currentColor, -1);
      } else {
        thisColor = minuteColor;
      }
    }

    // update all the pixels:
    strip.setPixelColor(pixel, thisColor);
  }
  strip.show();
}

void alarmMatch() {
  minuteReset = true;
}

// takes a color and fades its constituent parts by 1 point:
unsigned long fadeColor(unsigned long myColor, int difference) {
  byte firstPixel = myColor >> 16;
  byte secondPixel = myColor >> 8;
  byte thirdPixel = lowByte(myColor);
  firstPixel += difference;
  secondPixel += difference;
  thirdPixel += difference;
  firstPixel = max(firstPixel, 0);
  secondPixel = max(secondPixel, 0);
  thirdPixel = max(thirdPixel, 0);
  return (firstPixel << 16) | (secondPixel << 8) | thirdPixel;
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



