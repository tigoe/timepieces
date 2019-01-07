/*
  Hourglass controller
  This version uses APA102C LEDs in warm/cool/natural white
  and two pushbuttons to set hour and minute

  WiFi provisioning allows for network setup,
  and once connected, it obtains time from the network:
  created 23 Jun 2018
  modified 13 Nov 2018
  by Tom Igoe
*/
#include <SPI.h>
#include <RTCZero.h>
#include <Adafruit_DotStar.h>
#include <WiFi101.h>

#define DATAPIN    6
#define CLOCKPIN   5

// How many leds in your strip?
const int pixelCount = 60;

Adafruit_DotStar strip = Adafruit_DotStar( pixelCount, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
unsigned long secondColor = 0x0F0F0F;
unsigned long minuteColor = 0xFEFEFE;

int ledState = LOW;   // for the led state toggling

RTCZero rtc;
const int timeZone = -5; // your time zone relative to UTC
int thisMinute, thisSecond;

volatile int lastMinutePixel;

const int secondsButton = 0;
const int minutesButton = 1;
int lastmButton = HIGH;
bool fading = false;
int minutePixel, secondPixel = 0;

void setup() {
  Serial.begin(9600);
  pinMode(secondsButton, INPUT_PULLUP);
  pinMode(minutesButton, INPUT_PULLUP);

attachInterrupt(digitalPinToInterrupt(minutesButton), changeMinute, FALLING);

  // initialize the realtime clock:
  rtc.begin();
  rtc.setAlarmTime(00, 00, 00);     // set an alarm for processor wakeup
  rtc.enableAlarm(rtc.MATCH_SS);    // enable it
  rtc.attachInterrupt(alarmMatch);  // have the wakeup happen when the RTC seconds matches 00
  strip.begin();
  strip.clear();

  // Start in provisioning mode:
  WiFi.beginProvision();

  // while you're not connected to a WiFi AP,
  // try to connect:
  while (WiFi.status() != WL_CONNECTED) {
    // TODO add in an option to switch to standalone mode
    // when network not available, by pressing minute button
    int mButton = digitalRead(minutesButton);
    // exit the while loop:
    if (mButton == LOW) {
      // do the rest of setup, then exit:
      return;
    }
  }

  // When you're connected, print out the device's network status:
  IPAddress ip = WiFi.localIP();
  if (Serial) Serial.print("IP Address: ");
  if (Serial) Serial.println(ip);

  // get the time from the network:
  unsigned long epoch;
  do {
    epoch = WiFi.getTime();
  } while (epoch == 0);

  // adjust for time zone (in seconds) and set clock:
  epoch += (timeZone * 3600);
  rtc.setEpoch(epoch);

  // set all  initially:
  for (int pixel = 0; pixel < (59 - rtc.getSeconds()); pixel++) {
    // set the pixel to the new color
    strip.setPixelColor(pixel, secondColor);
  }
  strip.setPixelColor(59 - rtc.getMinutes(), minuteColor);
}

void loop() {
  if (thisSecond != rtc.getSeconds()) {
    if (Serial) Serial.println(getTimeStamp());
    thisSecond = rtc.getSeconds();
  }

  // convert to LED positions:
  secondPixel = 59 - thisSecond;
  minutePixel = 59 - thisMinute;

  // read seconds reset button:
  if (digitalRead(secondsButton) == LOW) {
    rtc.setSeconds(0);
  }

  fading = false;
  // at the top of the minute,fade everything up:
  if (rtc.getSeconds() == 0) {
    // fade all up
    for (int pixel = 0; pixel < pixelCount; pixel++) {
      unsigned long thisColor = strip.getPixelColor(pixel);
      if (thisColor < secondColor) {
        thisColor = fadeColor(thisColor, 1);
      } else if (thisColor > secondColor) {
        // fade last minute to secondColor
        thisColor = fadeColor(thisColor, -1);
      }
      // set the pixel to the new color
      strip.setPixelColor(pixel, thisColor);
    }
  } // end of getSeconds if statement

  // fade the previous second out:
  unsigned long secondPixelColor = strip.getPixelColor(secondPixel);
  if (secondPixelColor > 0) {
    secondPixelColor = fadeColor(secondPixelColor, -1);
    strip.setPixelColor(secondPixel, secondPixelColor);
  }

  // fade previous minute:
  unsigned long lastMinuteColor = strip.getPixelColor(lastMinutePixel);
  if (lastMinuteColor > secondColor) {
    lastMinuteColor = fadeColor(lastMinuteColor, -1);
    strip.setPixelColor(lastMinutePixel, lastMinuteColor);
  }

  // ensure that the minute stays up:
  unsigned long minutePixelColor = strip.getPixelColor(minutePixel);
  if (minutePixelColor < minuteColor) {
    minutePixelColor = fadeColor(minutePixelColor, 1);
    strip.setPixelColor(minutePixel, minutePixelColor);
  }

  if (fading) {
    strip.show();
  }
}

void changeMinute() {
  int m = rtc.getMinutes();
    m++;
    m = m % 60;
    rtc.setMinutes(m);
}

void alarmMatch() {
  //  minuteReset = true;
  lastMinutePixel = minutePixel;
  thisMinute = rtc.getMinutes();
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
  fading = true;
  return (firstPixel << 16) | (secondPixel << 8) | thirdPixel;
}

// assumes all three color values are the same
unsigned long fadeBrightness(unsigned long myColor, int difference) {
  byte colorVal = lowByte(myColor);
  colorVal = max(colorVal + difference, 0);
  fading = true;
  return (colorVal << 16) | (colorVal << 8) | colorVal;
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
