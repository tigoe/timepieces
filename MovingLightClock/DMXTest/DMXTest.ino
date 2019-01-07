/*
  DMX Test

  Tests an Elation moving spot

  Circuit:
   - DMX light
   - MKR board
   - MKR 485 shield
     - ISO GND connected to DMX light GND (pin 1)
     - Y connected to DMX light Data + (pin 2)
     - Z connected to DMX light Data - (pin 3)
     - Jumper positions
       - Z \/\/ Y set to ON

  created 5 July 2018
  by Sandeep Mistry
  modified 3 Jan 2019
  by Tom Igoe
*/

#include <ArduinoRS485.h> // the ArduinoDMX library depends on ArduinoRS485
#include <ArduinoDMX.h>

// set your highest DMX channel number here:
const int universeSize = 512;
// set the appropriate channels of the light you're controlling:
const int panChannel = 101;
const int tiltChannel = 103;
const int focusChannel = 110;
const int zoomChannel = 111;
const int shutterChannel = 113;
const int intensityChannel = 114;

int tilt, zoom, focus, intensity, shutter = 0;
int change = 1;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // initialize the DMX library with the universe size
  if (!DMX.begin(universeSize)) {
    Serial.println("Failed to initialize DMX!");
    while (1); // wait for ever
  }
  shutter = 255;
}

void loop() {
  setLight();


  tilt += change;
  zoom += change;
  focus += change;
  intensity += change;
  
  if (intensity == 0 || intensity == 255) {
    change = -change;
  }
  Serial.println(intensity);
  setLight();


  // delay for dimming effect
  delay(50);
}

void setLight() {
  DMX.beginTransmission();
  DMX.write(tiltChannel, tilt);
  DMX.write(zoomChannel, zoom);
  DMX.write(focusChannel, focus);
  DMX.write(intensityChannel, intensity);
  DMX.write(shutterChannel, shutter);
  DMX.endTransmission();
}
