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
boolean isPlaying = false;
// Lamp varialbes
boolean isLampOn = false;
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

bool playFirst() {
    execute_CMD(0x3F, 0, 0);
    delay(500);
    execute_CMD(0x11, 0, 1);
    delay(500);
    return true;
}

bool setPause() {
    execute_CMD(0x0E, 0, 0);
    delay(500);
    return true;
}

bool togglePlay() {
    if (isPlaying) {
        setPause();
    } else {
        playAll();
    }
    return true;
}

bool play() {
    execute_CMD(0x0D, 0, 1);
    delay(500);
    return true;
}

bool playAll() {
    execute_CMD(0x11, 0, 1);
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
    Particle.variable("volume", String(volume));
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

int dfMini(String command) {
    // Getting index of all the commas in the command
    int firstCommaIndex = command.indexOf(',');
    int secondCommaIndex = command.indexOf(',', firstCommaIndex+1);
    // Using that indexes to split the command subunits
    String function = command.substring(0, firstCommaIndex);
    String parameter = command.substring(firstCommaIndex+1, secondCommaIndex);
    // Printing to serial the received data
    Serial.print("\ndfMini command: " + command + "\n");
    Serial.print("function: " + function + "\n");
    Serial.print("parameter: " + parameter + "\n");
    // Execute the right function according to the one requested in the http call    
    if (function == "play") {
        if (playAll()) {
            isPlaying = true;
            Particle.publish("isPlaying", String(isPlaying), PRIVATE);        
            return 1;
        } else {
            return -1;
        }
    } else if (function == "pause") {
        if (setPause()) {
            isPlaying = false;
            Particle.publish("isPlaying", String(isPlaying), PRIVATE);        
            return 1;
        } else {
            return -1;
        }
    } else if (function == "playPrevious") {
        if (playPrevious()) {
            isPlaying = true;
            Particle.publish("isPlaying", String(isPlaying), PRIVATE);        
            return 1;
        } else {
            return -1;
        }
    } else if (function == "playNext") {
        if (playNext()) {
            isPlaying = true;
            Particle.publish("isPlaying", String(isPlaying), PRIVATE);        
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
    Serial.print("\nsetColor command: " + command + "\n");
    Serial.print("intensityRed: " + intensityRed + "\n");
    Serial.print("intensityGreen: " + intensityGreen + "\n");
    Serial.print("intensityBlue: " + intensityBlue + "\n");
    Serial.print("transitionMilliseconds: " + transitionMilliseconds + "\n");
    Serial.println();
    // Fading to new color
    fadeIn(intensityRed.toInt(), intensityGreen.toInt(), intensityBlue.toInt(), 10);
    //colorFade(intensityRed.toInt(), intensityGreen.toInt(), intensityBlue.toInt(), 10);
    if (intensityRed.toInt() == 0 &&  intensityGreen.toInt() == 0 && intensityBlue.toInt() == 0 ) {
        isLampOn = false;        
        Particle.publish("isLampOn", String(isLampOn), PRIVATE);
    } else {
        isLampOn = true;        
        Particle.publish("isLampOn", String(isLampOn), PRIVATE);
    }
    return 1;
}

void setup() {
    // Starting Neopixel LEDs
    strip.begin();
    // Initialize all pixels to 'off'
    strip.show(); 
    // Starting serial port used for debugging
    Serial.begin(9600);
    // Starting serial port used to control Df Mini Mp3 Player
    Serial1.begin(9600);
    // Publish functions on the Particle Cloud
    Particle.function("setColor", setColor);
    Particle.function("dfMini", dfMini);
    // Variables on the Particle Cloud
    Particle.variable("isLampOn", &isLampOn, BOOLEAN);
    Particle.variable("isPlaying", &isPlaying, BOOLEAN);
    // Music is not playing and lamp is off
    isPlaying = false;
    isLampOn = false;
    // And volume is set to Quiet
    setVolume(11);  
    // Mp3 Player buttons need to be set
    pinMode(buttonPause, INPUT_PULLUP);
    digitalWrite(buttonPause, HIGH);
    pinMode(buttonNext, INPUT_PULLUP);
    digitalWrite(buttonNext, HIGH);
    pinMode(buttonPrevious, INPUT_PULLUP);
    digitalWrite(buttonPrevious, HIGH);
    // if (Particle.connected()) {
    //     colorFade(0, 25, 0, 1000);
    //     colorFade(0, 0, 0, 1000);
    //     Particle.publish("Baby Sleep Training Lamp is now Cloud Connected", PUBLIC);
    //     Particle.variable("isLightOn", "true");
    // } else {
    //     colorFade(25, 0, 0, 1000);
    //     colorFade(0, 0, 0, 1000);
    //     colorFade(25, 0, 0, 1000);
    //     colorFade(0, 0, 0, 1000);
    //     colorFade(25, 0, 0, 1000);
    //     colorFade(0, 0, 0, 1000);
    //     Particle.variable("isLightOn", "false");
    // }
    
}

void loop() {
    // Particle.publishVitals();
    buttonInputChecker();
    //Particle.publish("Baby Sleep Training Lamp is now Cloud Connected", PUBLIC);
    //Particle.variable("isLightOn", "true");
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
