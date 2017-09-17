/**
* Little Cloud
* 
* Little Cloud is a WiFi controlled RGB lamp
* with Mp3 Player for baby sleep training
* 
* This is the Particle Photon code that controls its hardware
* 
* Davide Nastri, 8/20/2017
*/


/* ======================= includes ================================= */
// Neopixel RGB LED include
#include "Particle.h"
#include "neopixel.h"
// Http call include
#include "HttpClient/HttpClient.h" // Include HttpClient library
// Df Mini Mp3 Player includes
# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]
# define ACTIVATED LOW
// Df Mini Mp3 Player variables
int buttonNext = D3;
int buttonPause = D4;
int buttonPrevious = D5;
boolean isPlaying = true;
// HttpClient initialization begin
HttpClient http;
// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    
    { "Content-Type", "application/json" },
    { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL

};
http_request_t request;
http_response_t response;
// Declaring sendHttpRequest variables
unsigned int nextTime = 0; // Next time to contact the server
String method; // Currently only "get" and "post" are implemented
String hostname;
int port;
String path;

// Initializing all the previousIntensity variables to 0
int previousIntensityRed = 0;
int previousIntensityGreen = 0;
int previousIntensityBlue = 0;

/* ======================= prototypes =============================== */

void colorAll(uint32_t c, uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);
void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
uint32_t Wheel(byte WheelPos);

/* ======================= extra-examples.cpp ======================== */

SYSTEM_MODE(AUTOMATIC);

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 14
#define PIXEL_PIN D0
#define PIXEL_TYPE WS2812B

// Parameter 1 = number of pixels in strip
//               note: for some stripes like those with the TM1829, you
//                     need to count the number of segments, i.e. the
//                     number of controllers in your stripe, not the number
//                     of individual LEDs!
// Parameter 2 = pin number (most are valid)
//               note: if not specified, D2 is selected for you.
// Parameter 3 = pixel type [ WS2812, WS2812B, WS2812B2, WS2811,
//                             TM1803, TM1829, SK6812RGBW ]
//               note: if not specified, WS2812B is selected for you.
//               note: RGB order is automatically applied to WS2811,
//                     WS2812/WS2812B/WS2812B2/TM1803 is GRB order.
//
// 800 KHz bitstream 800 KHz bitstream (most NeoPixel products
//               WS2812 (6-pin part)/WS2812B (4-pin part)/SK6812RGBW (RGB+W) )
//
// 400 KHz bitstream (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//                   (Radio Shack Tri-Color LED Strip - TM1803 driver
//                    NOTE: RS Tri-Color LED's are grouped in sets of 3)

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.


void fadeIn(uint8_t red, uint8_t green, uint8_t blue, uint8_t wait) {

  for(uint8_t b=0; b <255; b++) {
     for(uint8_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, red*b/255, green*b/255, blue*b/255);
     }
     strip.show();
     delay(wait);
  };

};

void fadeOut(uint8_t red, uint8_t green, uint8_t blue, uint8_t wait) {

  for(uint8_t b=255; b > 0; b--) {
     for(uint8_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, red*b/255, green*b/255, blue*b/255);
     }
     strip.show();
     delay(wait);
  };

};

void fadeInAndOut(uint8_t red, uint8_t green, uint8_t blue, uint8_t wait) {

  for(uint8_t b=0; b <255; b++) {
     for(uint8_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, red*b/255, green*b/255, blue*b/255);
     }
     strip.show();
     delay(wait);
  };

  for(uint8_t b=255; b > 0; b--) {
     for(uint8_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, red*b/255, green*b/255, blue*b/255);
     }
     strip.show();
     delay(wait);
  };
};

void colorFade(uint8_t r, uint8_t g, uint8_t b, uint8_t wait) {
  for(uint16_t i = 0; i < strip.numPixels(); i++) {
      uint8_t startR, startG, startB;
      uint32_t startColor = strip.getPixelColor(i); // get the current colour
      startB = startColor & 0xFF;
      startG = (startColor >> 8) & 0xFF;
      startR = (startColor >> 16) & 0xFF;  // separate into RGB components

      if ((startR != r) || (startG != g) || (startB != b)){  // while the curr color is not yet the target color
        if (startR < r) startR++; else if (startR > r) startR--;  // increment or decrement the old color values
        if (startG < g) startG++; else if (startG > g) startG--;
        if (startB < b) startB++; else if (startB > b) startB--;
        strip.setPixelColor(i, startR, startG, startB);  // set the color
        strip.show();
        delay(1);  // add a delay if its too fast
      }
      delay(1000);
  }
}


void candle() {
   uint8_t green; // brightness of the green 
   uint8_t red;  // add a bit for red
   for(uint8_t i=0; i<100; i++) {
     green = 50 + random(155);
     red = green + random(50);
     strip.setPixelColor(random(strip.numPixels()), red, green, 0);
     strip.show();
     delay(5);
  }
}


int waitTime = 10;

// Set all pixels in the strip to a solid color, then wait (ms)
void colorAll(uint32_t c, uint8_t wait) {
  uint16_t i;

  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
  delay(wait);
}

// Fill the dots one after the other with a color, wait (ms) after each one
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout, then wait (ms)
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) { // 1 cycle of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}


// sendHttpRequest function
void sendHttpRequest(String method, String hostname, int port, String path, String body) {
    request.hostname = hostname;
    request.port = port;
    request.path = path;
    request.body = body;
    Serial.print("Method: " + method + "\n");
    Serial.print("Hostname: " + hostname + "\n");
    Serial.print("Port: " + String(port) + "\n");
    Serial.print("Path: " + path + "\n");
    Serial.print("Body: " + body + "\n");
    if (method == "post") {
        Serial.print("\nSending http request\n");
        http.post(request, response, headers);
        Serial.print("Application>\tResponse status: ");
        Serial.println(response.status);
        Serial.print("Application>\tHTTP Response Body: ");
        Serial.println(response.body);
      } else if (method == "get") {
        Serial.print("\nSending http request\n");
        http.get(request, response, headers);
        Serial.print("Application>\tResponse status: ");
        Serial.println(response.status);
        Serial.print("Application>\tHTTP Response Body: ");
        Serial.println(response.body);
      } else {
        Serial.print("Please check your method: only post and get are currently supported");
    }
}

int setColor(String command) {
    // Getting index of all the commas in the command
    int firstCommaIndex = command.indexOf(',');
    int secondCommaIndex = command.indexOf(',', firstCommaIndex+1);
    int thirdCommaIndex = command.indexOf(',', secondCommaIndex+1);
    // Using that indexes to split the command subunits
    String intensityRed = command.substring(0, firstCommaIndex);
    String intensityGreen = command.substring(firstCommaIndex+1, secondCommaIndex);
    String intensityBlue = command.substring(secondCommaIndex+1);
    String transitionMilliseconds = command.substring(thirdCommaIndex+1);
    // Printing to serial the color
    Serial.print("intensityRed: " + intensityRed);  
    Serial.println();
    Serial.print("intensityGreen: " + intensityGreen);  
    Serial.println();
    Serial.print("intensityBlue: " + intensityBlue);  
    Serial.println();
    Serial.print("transitionMilliseconds: " + transitionMilliseconds);  
    Serial.println();
    // Fading to new color
    fadeIn(intensityRed.toInt(), intensityGreen.toInt(), intensityBlue.toInt(), 10);
    return 1;
}

bool buttonInputChecker() {
    if (digitalRead(buttonPause) == ACTIVATED) {
        togglePlay();
    }
    if (digitalRead(buttonNext) == ACTIVATED) {
        if (isPlaying) {
            playNext();
        }
    }
    if (digitalRead(buttonPrevious) == ACTIVATED) {
        if (isPlaying) {
            playPrevious();
        }
    }
    return true;
}

bool togglePlay() {
    if (isPlaying) {
        pause();
    } else {
        playAll();
    }
    return true;
}

bool playFirst() {
    execute_CMD(0x3F, 0, 0);
    delay(500);
    execute_CMD(0x11, 0, 1);
    delay(500);
    return true;
}

bool pause() {
    execute_CMD(0x0E, 0, 0);
    isPlaying = false;
    delay(500);
    return true;
}

bool play() {
    execute_CMD(0x0D, 0, 1);
    isPlaying = true;
    delay(500);
    return true;
}

bool playAll() {
    execute_CMD(0x11, 0, 1);
    isPlaying = true;
    delay(500);
    return true;
}

bool playNext() {
    execute_CMD(0x01, 0, 1);
    delay(500);
    return true;
}

bool playPrevious() {
    execute_CMD(0x02, 0, 1);
    delay(500);
    return true;
}

bool setVolume(int volume) {
    execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
    delay(2000);
    return true;
}

// Excecute the command and parameters
bool execute_CMD(byte CMD, byte Par1, byte Par2) {
    // Calculate the checksum (2 bytes)
    int16_t checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
    // Build the command line
    byte Command_line[10] = {
        Start_Byte,
        Version_Byte,
        Command_Length,
        CMD,
        Acknowledge,
        Par1,
        Par2,
        checksum >> 8,
        checksum & 0xFF,
        End_Byte
    };
    //Send the command line to the module
    for (byte k = 0; k < 10; k++) {
        Serial1.write(Command_line[k]);
    }
    return true;
}

int dfMini(String command) {
    // Getting index of all the commas in the command
    int firstCommaIndex = command.indexOf(',');
    int secondCommaIndex = command.indexOf(',', firstCommaIndex+1);
    // Using that indexes to split the command subunits
    String function = command.substring(0, firstCommaIndex);
    String parameter = command.substring(firstCommaIndex+1, secondCommaIndex);
    // Printing to serial the received data
    Serial.print("function: " + function);  
    Serial.println();
    Serial.print("parameter: " + parameter);  
    Serial.println();
    // Execute the right function according to the one requested in the http call    
    if (function == "play") {
        if (play()) {
            return 1;
        } else {
            return -1;
        }
    } else if (function == "pause") {
        if (pause()) {
            return 1;
        } else {
            return -1;
        }
    } else if (function == "playPrevious") {
        if (playPrevious()) {
            return 1;
        } else {
            return -1;
        }
    } else if (function == "playNext") {
        if (playNext()) {
            return 1;
        } else {
            return -1;
        }
    } else if (function == "setVolume") {
        if (setVolume(parameter.toInt())) {
            return 1;
        } else {
            return -1;
        }
    }
}

void setup() {
    // Starting Neopixel LEDs
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    // Starting serial port used for debugging
    Serial.begin(9600);
    // Starting serial port used to control Df Mini Mp3 Player
    Serial1.begin(9600);
    // Publish functions on the Particle Cloud
    Particle.function("setColor", setColor);
    Particle.function("dfMini", dfMini);
    // Music is not playing
    isPlaying = false;
    // And volume is set to Max
    setVolume(20);  
    // Mp3 Player buttons need to be set
    pinMode(buttonPause, INPUT_PULLUP);
    digitalWrite(buttonPause, HIGH);
    pinMode(buttonNext, INPUT_PULLUP);
    digitalWrite(buttonNext, HIGH);
    pinMode(buttonPrevious, INPUT_PULLUP);
    digitalWrite(buttonPrevious, HIGH);
}

void loop() {
    buttonInputChecker();
    // Test LEDs
    //fadeIn(255, 0, 0, 10);
    //fadeIn(0, 255, 0, 10);
    //fadeIn(0, 0, 255, 10);
    //fadeOut(0, 0, 255, 5);
    //colorFade(255,0,0,1);
    //colorFade(0,255,0,1000);
    //colorFade(0,0,255,1);
    //candle();
    // Example http calls
    //sendHttpRequest("post", "davidenastri.it", 8080, "/", "Ciao");
    //sendHttpRequest("get", "davidenastri.it", 8080, "/", "Ciao");
}


/* DF Player mini command discovery (Modified for Particle world by @FiDel - Feb 16, 2016)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This program is meant to discover all the possibilities of the command structure of the DFPlayer mini.
No special libraries are needed.
It's very easy to understand and can be the basis for your own mp3 player sketch.
Note: Commands are not always correctly described in the manual. I tried to fix it, but there is still a lot to do. The commands recoverd so far are listed below.

Use of sketch: Enter three (separated) decimal numbers in the Serial Monitor with no end of line character.
First number : Command
Second number: First (High Byte) parameter
Third number : Second (Low Byte) parameter
E.g.: 3,0,1 will play the first track on the TF card

Very important for 5V Arduinos: Use serial 1K resistors or a level shifter between module RX and TX and Arduino to suppress noise
Connect Sound module board RX to Arduino pin 11 (via 1K resistor)
Connect Sound module board TX to Arduino pin 10 (via 1K resistor)
Connect Sound module board Vcc to Arduino Vin when powered via USB (preferably 3.0) else use seperate 5V power supply
Connect Sound module board GND to Arduino GND

General DF Player mini command structure (only byte 3, 5 and 6 to be entered in the serial monitor):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Byte Function Value
==== ================ ====
(0) Start Byte 0x7E
(1) Version Info 0xFF (don't know why it's called Version Info)
(2) Number of bytes 0x06 (Always 6 bytes)
(3) Command 0x__
(4) Command feedback 0x__ If enabled returns info with command 0x41 [0x01: info, 0x00: no info]
(5) Parameter 1 [DH] 0x__
(6) Parameter 2 [DL] 0x__
(7) Checksum high 0x__ See explanation below. Is calculated in function: execute_CMD
(8) Checksum low 0x__ See explanation below. Is calculated in function: execute_CMD
(9) End command 0xEF

Checksum calculation.
~~~~~~~~~~~~~~~~~~~~
Checksum = -Sum(byte(1..6)) (2 bytes, notice minus sign!)

Commands without returned parameters (*=Confirmed command ?=Unknown, not clear or not validated)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CMD CMD
HEX Dec Function Description Parameters(2 x 8 bit)
==== === =================================== ========================================================================
0x01 1 Next * [DH]=X, [DL]=X Next file in current folder.Loops when last file played
0x02 2 Previous * [DH]=X, [DL]=X Previous file in current folder.Loops when last file played
0x03 3 Specify track(NUM) * [DH]=highByte(NUM), [DL]=lowByte(NUM)
1~2999 Playing order is order in which the numbers are stored.
Filename and foldername are arbitrary, but when named starting with
an increasing number and in one folder, files are played in
that order and with correct track number.
e.g. 0001-Joe Jackson.mp3...0348-Lets dance.mp3)
0x04 4 Increase volume * [DH]=X, [DL]=X Increase volume by 1
0x05 5 Decrease volume * [DH]=X, [DL]=X Decrease volume by 1
0x06 6 Specify volume * [DH]=X, [DL]= Volume (0-0x30) Default=0x30
0x07 7 Specify Equalizer * [DH]=X, [DL]= EQ(0/1/2/3/4/5) [Normal/Pop/Rock/Jazz/Classic/Base]
0x08 8 Specify repeat(NUM) * [DH]=highByte(NUM), [DL]=lowByte(NUM).Repeat the specified track number
0x09 9 Specify playback source (Datasheet) ? [DH]=X, [DL]= (0/1/2/3/4)Unknown. Seems to be overrided by automatic detection
(Datasheet: U/TF/AUX/SLEEP/FLASH)
0x0A 10 Enter into standby – low power loss * [DH]=X, [DL]=X Works, but no command found yet to end standby
(insert TF-card again will end standby mode)
0x0B 11 Normal working (Datasheet) ? Unknown. No error code, but no function found
0x0C 12 Reset module * [DH]=X, [DL]=X Resets all (Track = 0x01, Volume = 0x30)
Will return 0x3F initialization parameter (0x02 for TF-card)
"Clap" sound after excecuting command (no solution found)
0x0D 13 Play * [DH]=X, [DL]=X Play current selected track
0x0E 14 Pause * [DH]=X, [DL]=X Pause track
0x0F 15 Specify folder and file to playback * [DH]=Folder, [DL]=File
Important: Folders must be named 01~99, files must be named 001~255
0x10 16 Volume adjust set (Datasheet) ? Unknown. No error code. Does not change the volume gain.
0x11 17 Loop play * [DH]=X, [DL]=(0x01:play, 0x00:stop play)
Loop play all the tracks. Start at track 1.
0x12 18 Play mp3 file [NUM] in mp3 folder * [DH]=highByte(NUM), [DL]=lowByte(NUM)
Play mp3 file in folder named mp3 in your TF-card. File format exact
4-digit number (0001~2999) e.g. 0235.mp3
0x13 19 Unknown ? Unknown: Returns error code 0x07
0x14 20 Unknown ? Unknown: Returns error code 0x06
0x15 21 Unknown ? Unknown: Returns no error code, but no function found 
0x16 22 Stop * [DH]=X, [DL]=X, Stop playing current track
0x17 23 Loop Folder 01 * [DH]=x, [DL]=1~255, Loops all tracks in folder named "01"
0x18 24 Random play * [DH]=X, [DL]=X Random all tracks, always starts at track 1
0x19 25 Single loop * [DH]=0, [DL]=0 Loops the track that is playing
0x1A 26 Pause * [DH]=X, [DL]=(0x01:pause, 0x00:stop pause)

Commands with returned parameters (*=Confirmed command ?=Unknown, not clear or not validated)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CMD CMD
HEX Dec Function Description Parameters(2 x 8 bit)
==== === =================================== ===========================================================================
0x3A 58 Medium inserted * [DH]=0, [DL]=(1:U-disk, 2:TF-card)
0x3B 59 Medium ejected * [DH]=0, [DL]=(1:U-disk, 2:TF-card)
0x3C 60 Finished track on U-disk * [DH]=highByte(NUM), [DL]=lowByte(NUM)
Not validated. Returns track number when song is finished on U-Disk
0x3D 61 Finished track on TF-card * [DH]=highByte(NUM), [DL]=lowByte(NUM)
Returns track number when song is finished on TF
0x3E 62 Finished track on Flash * [DH]=highByte(NUM), [DL]=lowByte(NUM)
Not validated. Returns track number when song is finished on Flash
0x3F 63 Initialization parameters * [DH]=0, [DL]= 0 ~ 0x0F. Returned code when Reset (0x12) is used.
(each bit represent one device of the low-four bits)
See Datasheet. 0x02 is TF-card. Error 0x01 when no medium is inserted.
0x40 64 Error ? [DH]=0, [DL]= 0~7 Error code(Returned codes not yet analyzed)
0x41 65 Reply ? [DH]=0, [DL]= 0~? Return code when command feedback is high
0x00 no Error (Other returned code not known)
0x42 66 The current status * [DH] = Device number [DL] = 0 no play, 1 play
0x43 67 The current volume * [DH]=0, [DL]= Volume (0-30)
0x44 68 The current EQ * [DH]=0, [DL]= EQ(0/1/2/3/4/5) [Normal/Pop/Rock/Jazz/Classic/Base]
0x45 69 The current playback mode * [DH]=0, [DL]= (0x00: no CMD 0x08 used, 0x02: CMD 0x08 used, not usefull)
0x46 70 The current software version * [DH]=0, [DL]= Software version. (My version is 5)
0x47 71 The total number of U-disk files * [DH]=highByte(NUM), [DL]=lowByte(NUM). Not validated
0x48 72 The total number of TF-card files * [DH]=highByte(NUM), [DL]=lowByte(NUM)
0x49 73 The total number of flash files * [DH]=highByte(NUM), [DL]=lowByte(NUM). Not validated
0x4A 74 Keep on (Datasheet) ? Unknown. No returned parameter
0x4B 75 The current track of U-Disk * [DH]=highByte(NUM), [DL]=lowByte(NUM), Current track on all media
0x4C 76 The current track of TF card * [DH]=highByte(NUM), [DL]=lowByte(NUM), Current track on all media
0x4D 77 The current track of Flash * [DH]=highByte(NUM), [DL]=lowByte(NUM), Current track on all media
0x4E 78 Folder "01" [DH]=x, [DL]=1 * [DH]=0, [DL]=(NUM) Change to first track in folder "01"
Returns number of files in folder "01"
0x4F 79 The total number of folders * [DH]=0, [DL]=(NUM), Total number of folders, including root directory

Additional info can be found on DFRobot site, but is not very reliable
Additional info:http://www.dfrobot.com/index.php?route=product/product&product_id=1121

Ype Brada 2015-04-06
*/
