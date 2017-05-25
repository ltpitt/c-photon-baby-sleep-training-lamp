/**
* Little Cloud
* 
* Little Cloud is a WiFi controlled RGB lamp
* with Mp3 Player for baby sleep training
* 
* This is the Particle Photon code that controls its hardware
* 
* Davide Nastri, 2017
*/

// RGB LED include
#include <rgb-controls.h> // Include rgb led control library
using namespace RGBControls;
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
// Declaring RGB led variables
// Connect Red to D3
// Connect Green to D1
// Connect Blue to D0
// Create a common anode led using false
Led led(D0, D1, D2, false);
Color red(255, 0, 0);
Color green(0, 255, 0);
Color blue(0, 0, 255);
Color black(0, 0, 0);

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

bool setColor(String command) {
    // Getting index of all the commas in the command
    int firstCommaIndex = command.indexOf(',');
    int secondCommaIndex = command.indexOf(',', firstCommaIndex+1);
    // Using that indexes to split the command subunits
    String intensityRed = command.substring(0, firstCommaIndex);
    String intensityGreen = command.substring(firstCommaIndex+1, secondCommaIndex);
    String intensityBlue = command.substring(secondCommaIndex+1);
    // Printing to serial the color
    Serial.print("intensityRed: " + intensityRed);  
    Serial.println();
    Serial.print("intensityGreen: " + intensityGreen);  
    Serial.println();
    Serial.print("intensityBlue: " + intensityBlue);  
    Serial.println();
    // Color variable
    Color color(intensityRed.toInt(), intensityGreen.toInt(), intensityBlue.toInt());
    // Setting the LED to the specified color
    led.setColor(Color(color));
    return true;
}



bool fadeColor(String command) {
    // Getting index of all the commas in the command
    int firstCommaIndex = command.indexOf(',');
    int secondCommaIndex = command.indexOf(',', firstCommaIndex+1);
    int thirdCommaIndex = command.indexOf(',', secondCommaIndex+1);
    int fourthCommaIndex = command.indexOf(',', thirdCommaIndex+1);
    int fifthCommaIndex = command.indexOf(',', fourthCommaIndex+1);
    int sixthCommaIndex = command.indexOf(',', fifthCommaIndex+1);
    // Using that indexes to split the command subunits
    String intensityRedStart = command.substring(0, firstCommaIndex);
    String intensityGreenStart = command.substring(firstCommaIndex+1, secondCommaIndex);
    String intensityBlueStart = command.substring(secondCommaIndex+1, thirdCommaIndex);
    String intensityRedEnd = command.substring(thirdCommaIndex+1, fourthCommaIndex);
    String intensityGreenEnd = command.substring(fourthCommaIndex+1, fifthCommaIndex);
    String intensityBlueEnd = command.substring(fifthCommaIndex+1, sixthCommaIndex);
    String transitionMilliseconds = command.substring(sixthCommaIndex+1);
    // Printing to serial the start color
    Serial.print("intensityRedStart: " + intensityRedStart);  
    Serial.println();
    Serial.print("intensityGreenStart: " + intensityGreenStart);  
    Serial.println();
    Serial.print("intensityBlueStart: " + intensityBlueStart);  
    Serial.println();
    // Printing to serial the end color    
    Serial.print("intensityRedEnd: " + intensityRedEnd);  
    Serial.println();
    Serial.print("intensityGreenEnd: " + intensityGreenEnd);  
    Serial.println();
    Serial.print("intensityBlueEnd: " + intensityBlueEnd);  
    Serial.println();
    Serial.print("transitionMilliseconds: " + transitionMilliseconds);  
    Serial.println();
    // Start color variable
    Color start(intensityRedStart.toInt(), intensityGreenStart.toInt(), intensityBlueStart.toInt());
    // End color variable
    Color end(intensityRedEnd.toInt(), intensityGreenEnd.toInt(), intensityBlueEnd.toInt());
    // Fade one from Start color to End color
    led.fadeOnce(start, end, transitionMilliseconds.toInt());
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

bool togglePlay() {
    if (isPlaying) {
        pause();
    } else {
        play();
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

bool dfMini(String command) {
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
        play();
    } else if (function == "pause") {
        pause();        
    } else if (function == "playPrevious") {
        playPrevious();
    } else if (function == "playNext") {
        playNext();
    } else if (function == "setVolume") {
        setVolume(parameter.toInt());
    }
    return true;
}


void setup() {
    // Starting serial port used for debugging
    Serial.begin(9600);
    // Starting serial port used to control Df Mini Mp3 Player
    Serial1.begin(9600);
    // Publish functions on the Particle Cloud
    Particle.function("fadeColor", fadeColor);
    Particle.function("setColor", setColor);
    Particle.function("dfMini", dfMini);
    // When the Little Cloud is turned on the LED is off
    led.off();
    // Music is not playing
    isPlaying = false;
    // And volume is set to Max
    setVolume(30);  
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
    // Pulse red from 0% brightness to 100% brightness every 5 seconds
    //led.fade(red.withBrightness(10), red, 5000);
    // Example http calls
    //sendHttpRequest("post", "davidenastri.it", 8080, "/", "Ciao");
    //sendHttpRequest("get", "davidenastri.it", 8080, "/", "Ciao");
}
