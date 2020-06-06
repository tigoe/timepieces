/*
  Moving Light Clock

  Moves an Elation Platinum Spot LED Pro II through its tilt
  starting at sunrise and ending at sunset. To do this, the
  sketch gets the time using the WiFi101 library's getTime() command,
  then gets the sunrise and sunset from the sunrise-sunset.org api.
  With that info, it calculates the day length and sends DMX commands
  to the spotlight to move it proportionally through the day

  Circuit:
    Arduino MKR1000
    RS485 Shield for MKR series
    ELation Elation Platinum Spot LED Pro II moving DMX spotlight

  created 20 Oct 2018
  modified 6 Jan 2019
  by Tom Igoe
*/

#include <ArduinoRS485.h>
#include <ArduinoDMX.h>
#include <SPI.h>
#include <WiFi101.h>
//#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <RTCZero.h>
#include <Scheduler.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"

// change the values of these two in the arduino_serets.h file:
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

RTCZero rtc;              // the realtime clock instance
long dayLength;           // length of today
int sunrise[] = {0, 0, 0};// sunrise h, m, s
int sunset[] = {0, 0, 0}; // sunset h, m, s
int noon[] = {0, 0, 0};   // midday h, m, s
int timeZone = -5;  // your time zone relative to UTC

// your latitude and longitude:
const float latitude = 40.73;
const float longitude = -73.99;

// set your highest DMX channel number here:
const int universeSize = 512;
// set the appropriate channels of the light you're controlling:
const int panChannel = 101;
const int tiltChannel = 103;
const int focusChannel = 110;
const int zoomChannel = 111;
const int shutterChannel = 113;
const int intensityChannel = 114;

// DMX values:
byte pan = 0;
byte tilt = 0;
byte zoom = 0;
byte focus = 33; // chosen randomly because my light looks good this way
byte intensity = 0;
byte shutter = 255;

WiFiClient netSocket;                                     // network socket to server
const char server[] = "api.sunrise-sunset.org";           // server name
const String route = "/json?lat=LAT&lng=LONG&date=today"; // API route

void setup() {
  Serial.begin(9600);
  delay(2000);
  // initialize the realtime clock:
  rtc.begin();

  // while you're not connected to a WiFi AP,
  // try to connect:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);   // print the network name (SSID)
    WiFi.begin(ssid, pass); // try to connect
    delay(2000);
  }

  // When you're connected, print out the device's network status:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // get the time from the network:
  unsigned long epoch;
  do {
    Serial.println("Trying to get time...");
    epoch = WiFi.getTime();
    delay(500);
  } while (epoch == 0);

  // print the epoch once you have it:
  Serial.print("Epoch received: ");
  Serial.println(epoch);
  rtc.setEpoch(epoch);

  // get the day of the week:
  // Jan 1 1970 (epoch) was a Thursday, so add 4:
  int dayOfWeek = ((epoch / 86400) + 4) % 7;

  // if daylight savings, spring forward:
  if (daylightSavings(dayOfWeek)) {
    timeZone = timeZone + 1;
  }

  Serial.println(timeZone);
  rtc.setHours(rtc.getHours() + timeZone);

  // print the time:
  Serial.println(getTimeString(rtc.getHours(),
                               rtc.getMinutes(),
                               rtc.getSeconds()));
  // get the day length:
  getDayLength();

  // initialize DMX:
  if (!DMX.begin(universeSize)) {
    Serial.println("Failed to initialize DMX!");
    while (1); // wait forever
  }

  // start the DMX update loop so the light gets a signal every 100ms:
  Scheduler.startLoop(setLight);
}

void loop() {
  // get the time:
  int hour = rtc.getHours();
  int minute = rtc.getMinutes();
  int second = rtc.getSeconds();

  // if it's 00:00:00:
  if (hour == 0 && minute == 0 && second == 0) {
    getDayLength();
  }

  // if it's sunrise:
  if (hour == sunrise[0] &&
      minute == sunrise[1] &&
      second == sunrise[2]) {
    // open the shutter:
    shutter = 255;
  }

  // if it's sunset:
  if (hour == sunset[0] &&
      minute == sunset[1] &&
      second == sunset[2]) {
    // reset the light for tomorrow:
    tilt = 0;
    zoom = 0;
    focus = 0;
    intensity = 0;
    shutter = 0;
  }

  // convert time to angle and intensity:
  intensity = getIntensity();
  zoom = 255 - intensity; // zoom most open at noon
  tilt = getAngle();

  // yield for the DMX loop to function too:
  yield();
}

// get the time as a string HH:MM:SS:
String getTimeString(int h, int m, int s) {
  String now = "";
  if (h < 0) now += "0";
  now += h;
  now += ":";
  if (m < 0) now += "0";
  now += m;
  now += ":";
  if (s < 0) now += "0";
  now += s;
  return now;
}

// calculate intensity as distance from solar noon.
// brightest at noon, dimmest at sunrise and sunset:
int getIntensity() {
  // distance from solar noon:
  float result = (noon[0] * 3600 +
                  noon[1] * 60 +
                  noon[2]) -
                 (rtc.getHours() * 3600 +
                  rtc.getMinutes() * 60 +
                  rtc.getSeconds());

  result = abs(result / (dayLength / 2)); // convert to 0-1
  result = 255 - (result * 255);     // get as a % of 255
  return byte(result);
}

// calculate angle as distance from sunrise:
int getAngle() {
  // time from sunset in seconds
  float result = (sunset[0] * 3600 +
                  sunset[1] * 60 +
                  sunset[2]) -
                 (rtc.getHours() * 3600 +
                  rtc.getMinutes() * 60 +
                  rtc.getSeconds());

  result = result / dayLength; //  convert to 0-1
  result = (result * 255);     // get as a % of 255
  return byte(result);
}

// send DMX values to light
//(this is a separate Scheduler loop):
void setLight() {
  DMX.beginTransmission();
  DMX.write(tiltChannel, tilt);
  DMX.write(zoomChannel, zoom);
  DMX.write(focusChannel, focus);
  DMX.write(intensityChannel, intensity);
  DMX.write(shutterChannel, shutter);
  DMX.endTransmission();
  delay(100);
  yield();
}

// calculate day length from network data:
void getDayLength() {
  // copy the route template:
  String thisRoute = route;
  // replace latitude and longitude:
  thisRoute.replace("LAT", String(latitude));
  thisRoute.replace("LONG", String(longitude));
  // get the sunrise and sunset from the web:
  HttpClient http(netSocket, server); // make an HTTP client
  http.get(thisRoute);                // make a GET request

  /*
    Get daytime data from sunrise-sunset.org api.
    Data is returned in a JSON string, and times are given
    in UTC, formatted in AM/PM format, so you need to adjust
    using your timezone and +12 for noon and after:
  */

  while (http.connected()) {
    // while connected to the server,
    if (http.available()) {
      // if there is a response from the server,
      // search the string until the word "sunrise",
      // then the next three numbers will be hours, mins, secs
      if (http.find("sunrise")) {
        sunrise[0] = http.parseInt() + timeZone;
        sunrise[1] = http.parseInt();
        sunrise[2] = http.parseInt();
      }
      // search the string until the word "sunset",
      // then the next three numbers will be hours, mins, secs
      if (http.find("sunset")) {
        sunset[0] = http.parseInt() + timeZone + 12;
        sunset[1] = http.parseInt();
        // API time is given in AM/PM, so add 12 hours for sunset:
        sunset[2] = http.parseInt();
      }
      // search the string until the word "solar_noon",
      // then the next three numbers will be hours, mins, secs
      if (http.find("solar_noon")) {
        noon[0] = http.parseInt() + timeZone + 12;
        noon[1] = http.parseInt();
        // API time is given in AM/PM, so add 12 hours for sunset:
        noon[2] = http.parseInt();
      }
      // search the string until the word "day_length",
      // then the next three numbers will be hours, mins, secs
      if (http.find("day_length")) {
        int dayHours = http.parseInt();
        int dayMinutes = http.parseInt();
        int daySeconds = http.parseInt();

        dayLength = dayHours * 3600 + dayMinutes * 60 + daySeconds;
      }
    }
  }
  // when there's nothing left to the response,
  http.stop(); // close the request
}



bool daylightSavings(int dow) {
  //January, february, and december are out.
  if (rtc.getMonth() < 3 || rtc.getMonth() > 11) {
    return false;
  }
  //April to October are in
  if (rtc.getMonth() > 3 && rtc.getMonth() < 11) {
    return true;
  }
  int previousSunday = rtc.getDay() - dow;
  //In march, we are DST if our previous sunday was on or after the 8th.
  if (rtc.getMonth() == 3) {
    return previousSunday >= 8;
  }
  //In november we must be before the first sunday to be dst.
  //That means the previous sunday must be before the 1st.
  return previousSunday <= 0;
}
