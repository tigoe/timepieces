// Email Clock 0002
//    Microcontroller is connected to a Lantronix Xport
//    serial-to-ethernet device. This program connects
//    to a HTTP server through the Xport, makes a HTTP GET
//    request for a PHP script, and sends the value
//    of an analog sensor through as a part of the
//    GET request.

//    Xport communicates at 9600-8-n-1 non-inverted (true) serial.

//    By Tom Igoe, 31 oct 2005
//    updated 13 Dec. 2005

// Defines for the Xport's status (used for staus variable):
#define disconnected 0
#define connected 1
#define connecting 2
#define requesting 3
#define reading 4
/*
  Note: Status LEDs correspond to status states, as follows:
  disconnected LED:  Arduino digital I/O 6
  connected LED:  Arduino digital I/O 7
  connecting LED:  Arduino digital I/O 8
  requesting LED:  Arduino digital I/O 9
*/

// Defines for I/O pins:
#define xportResetPin 10
#define clockPin1 11
#define clockPin2 12


// Define for clock tick interval, in ms:
#define interval 20

// variables:
int inByte= -1;          // incoming byte from serial RX
char inString[32];       // string for incoming serial data
int stringPos = 0;       // string index counter
int kilobytes = 0;       // number of kilobytes of mail

int status = 0;          // Xport's connection status
int secs = 0;            // second counter (used to sleep between checks)

// Function prototypes:
void tick();                         // makes the clock tick forward
void xportConnect();                 // opens TCP connection
void httpRequest();                  // makes HTTP request
void resetXport();                   // resets Xport
void printResults();                 // for debugging only
void blink(int howManyTimes);        // blinks an LED on reset
void countDown(int numberOfTicks);   // counts down kilobytes of email

void setup() {
int i = 0;               // generic loop counter
    // set all status LED pins and Xport reset pin:
  for (i = 6; i < 11; i++) {
    pinMode(i, OUTPUT);
  }
  
  // set up clock pins as outputs:
  pinMode(clockPin1, OUTPUT);
  pinMode(clockPin2, OUTPUT);

  // set up reset LED pin as output:
  pinMode(13, OUTPUT);
  
  // start serial port, 9600 8-N-1:
  beginSerial(9600);
  
  // blink reset LED and reset Xport:
  blink(3);
  resetXport();
}

void loop() {
int i = 0;               // generic loop counter

    // set the status lights:
  for (i = 6; i < 10; i++) {
    if (status == i - 6) {
      digitalWrite(i, HIGH);
    } 
    else {
      digitalWrite(i, LOW);
    }
  }

  // if you're connected to the server,  make a HTTP call.  
  // If not, connect to the server:

  if(status == disconnected) {
    // attempt to connect to the server:
    xportConnect();
  } 
  
  if (status == connecting) {
    // read the serial port:
    if (serialAvailable()) {
      inByte = serialRead();
      printByte(inByte);
      if (inByte == 67) {  // 'C' in ascii
        status = connected;    
      }
    }

  }
  if (status == connected) {
    // send HTTP GET request for CGI script:
    httpRequest();
  }  

  if (status == requesting) {
    // wait for bytes from server:
    // read the serial port:
    if (serialAvailable()) {
      inByte = serialRead();
      // If you get a "<", what follows is the kilobyte count:
      if (inByte == 60) {
        stringPos = 0;
        status = reading;
      }
    }
  }

  if (status == reading) {
    if (serialAvailable()) {
      inByte = serialRead();
// Keep reading until you get a ">":
      if (inByte != 62) {
        // save only ASCII numeric characters:
        if ((inByte >= 48) && (inByte <= 57)){
          inString[stringPos] = inByte;
          stringPos++;
        }
      } 
      else {
        // convert the string to a numeric value:
        kilobytes = atoi(inString);
        
        // Tick down the number of kilobytes:
        countDown(kilobytes);
        status = disconnected;
        
        // wait 60 seconds before trying again:
        for (secs = 0; secs < 60; secs++) {
          delay(1000);
        }
        
        // reset Xport before next request:
        resetXport();
      }
    }
  }
}

void xportConnect() {
  //   send out the server address and 
  //   wait for a "C" byte to come back.
  //   fill in your server's numerical address below:
  printString("C192.168.42.23/80\n");
  status = connecting;
}

void httpRequest() {
int i = 0;               // generic loop counter
  inByte = -1;  
  stringPos = 0;
  //  Make HTTP GET request. Fill in the path to your version
  //  of the CGI script:
  printString("GET /~accountname/cgi-bin/emailclock01.cgi HTTP/1.1\n");
  delay(250);
  //  Fill in your server's name:
  printString("HOST: myserver.com\n\n");
  status = requesting;
}

void printResults() {
// this routine used in debugging only, to print out the results:
  printString(" I got ");
  printInteger(stringPos);
  if (stringPos > 0) {
    printString(" bytes, total:  ");
    printInteger(kilobytes); 
    printString(" string: " );
    for (i = 0; i<stringPos; i++) {
      printByte(inString[i]);
    }  
  }
  printString("\n\n\n");
  tick();
}

// Take the Xport's reset pin low to reset it:
void resetXport() {
  digitalWrite(xportResetPin, LOW);
  delay(50);
  digitalWrite(xportResetPin, HIGH);
  // pause to let Xport boot up:
  delay(2000);
}

// Blink the reset LED:
void blink(int howManyTimes) {
  int i;
  for (i=0; i< howManyTimes; i++) {
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(200);  
  }
}

// Count down the number of ticks, one for each kilobyte of mail:
void countDown(int numberOfTicks) {
  int ticksLeft = numberOfTicks;
  while (ticksLeft) {
    tick();
    ticksLeft--;
    delay(interval * 5);
  }
}
/* 
 Each tick moves the clock forward about two seconds.  
 It's a bit of a hack, but gets the point across.
 */

void tick() {
  digitalWrite(clockPin1, HIGH);
  digitalWrite(clockPin2, LOW);
  delay(interval);
  digitalWrite(clockPin1, LOW);
  digitalWrite(clockPin2, HIGH);
  delay(interval);
  digitalWrite(clockPin1, HIGH);
  delay(interval);
  digitalWrite(clockPin1, LOW);
  digitalWrite(clockPin2, LOW);
  delay(interval);
}
