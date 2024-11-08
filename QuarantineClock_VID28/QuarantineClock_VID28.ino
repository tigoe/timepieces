
/*
  Quarantine Clock

  Controls two stepper motors at once, used to test a VID28-05
  concentric shaft motor. Treats the inner hand as a minute hand,
  and the outer as a second hand.

  Time stays on the same second in this sketch.

  Created 20 Jan 2020
  modified 15 April 2020
  by Tom Igoe

*/
#include <RTCZero.h>
#include <Stepper.h>
const int stepsPerRevolution = 720;  // VID28-05 is 720 steps per revolution

// initialize the stepper library.
// Any 8 pins will do. These numbers were used for the MKR Zero form factor:
Stepper secondMotor(stepsPerRevolution, A5, A6, 0, 1);
Stepper minuteMotor(stepsPerRevolution, 2, 3, 4, 5);

int secondSteps = 0;
int minuteSteps = 0;
// motors will move in opposite directions:
int secondDir = 1;
int minuteDir = 1;

RTCZero rtc;
int lastSecond = -1;  // the previous second
int lastMinute = -1;  // the previous minute
long lastMove = 0;    // last time the hands were moved
int speedMillis = 81; // 0.08333s per step, approx.
int s = 12;
int t = -13;
int m = s;
int n = t;

void setup() {
  // initialize the serial port:
  Serial.begin(9600);
  // initialize the RTC:
  rtc.begin();
  rtc.setTime(0, 0, 0);
  rtc.setAlarmTime(0, 0, 0);
  rtc.enableAlarm(rtc.MATCH_SS);
  rtc.attachInterrupt(moveMinute);

}

void loop() {
  int thisSecond = rtc.getSeconds();
  if (thisSecond != lastSecond) {
    Serial.println(thisSecond);
    secondMotor.step(s);
    delay(50);
    secondMotor.step(t);
    s = -s;
    t = -t;
    lastSecond = thisSecond;
  }
}

void moveMinute() {
  minuteMotor.step(m);
  delay(50);
  minuteMotor.step(n);
  m = -m;
  n = -n;
}
