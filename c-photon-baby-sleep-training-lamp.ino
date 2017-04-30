/**
* SendHttpRequest Function and Example
*/

#include <rgb-controls.h> // Include rgb led control library
using namespace RGBControls;
#include "HttpClient/HttpClient.h" // Include HttpClient library

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
// HttpClient initialization end

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
Led led(D3, D1, D0, false);
Color red(255, 0, 0);
Color green(0, 255, 0);
Color blue(0, 0, 255);
Color black(0, 0, 0);


// sendHttpRequest function
void sendHttpRequest(String method, String hostname, int port, String path, String body)
{
    
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

bool setColor(String command)
{
    int firstCommaIndex = command.indexOf(',');
    int secondCommaIndex = command.indexOf(',', firstCommaIndex+1);
    String intensityRed = command.substring(0, firstCommaIndex);
    String intensityGreen = command.substring(firstCommaIndex+1, secondCommaIndex);
    String intensityBlue = command.substring(secondCommaIndex+1);
    Serial.print("intensityRed: " + intensityRed);  
    Serial.println();
    Serial.print("intensityGreen: " + intensityGreen);  
    Serial.println();
    Serial.print("intensityBlue: " + intensityBlue);  
    Serial.println();
    led.setColor(Color(intensityRed.toInt(), intensityGreen.toInt(), intensityBlue.toInt()));
    return True;
}



bool fadeColor(String command)
{

    //led.fade(red, blue, 3000);    
    //delay(5000);
    int firstCommaIndex = command.indexOf(',');
    int secondCommaIndex = command.indexOf(',', firstCommaIndex+1);
    int thirdCommaIndex = command.indexOf(',');
    int fourthCommaIndex = command.indexOf(',', secondCommaIndex+1);

    String intensityRedFirst = command.substring(0, firstCommaIndex);
    String intensityGreenFirst = command.substring(firstCommaIndex+1, secondCommaIndex);
    String intensityBlueFirst = command.substring(secondCommaIndex+1);
    Serial.print("intensityRed: " + intensityRed);  
    Serial.println();
    Serial.print("intensityGreen: " + intensityGreen);  
    Serial.println();
    Serial.print("intensityBlue: " + intensityBlue);  
    Serial.println();
    return True;
    // Put into commaIndex variable comma character index number, in order to use it later to split the string
    int commaIndex = command.indexOf(',');
// Use the comma separator index to take the 1st part of the command (containing Photon pin port number connected to Relay)
    String pinNumber = command.substring(0, commaIndex);
// Use the comma separator index to take the 2nd part of the command (containing the command being sent e.g. on, off, on_off etc)
    String relayCommand = command.substring(commaIndex+1);
// Convert the pinNumber from string to integer in order to successfully issue the digitalWrite command later on
    int pinNumberI = pinNumber.toInt();
// Print both values to serial for debugging purpose
//    Serial.print("Pin Number: " + pinNumber);  
//    Serial.println();
    Serial.print("Relay Command: " + relayCommand);  
    Serial.println();
}
    


void setup() {
    Serial.begin(9600);
    // Publish functions on the Particle Cloud
    Particle.function("fadeColor", fadeColor);
    Particle.function("setColor", setColor);
}

void loop() {
    // Pulse red from 0% brightness to 100% brightness every 5 seconds
    //led.fade(red.withBrightness(10), red, 5000);
    //led.fade(red, blue, 5000);
    //led.fade(red, blue, 3000);    
    //delay(5000);
    //delay(5000);
    //led.setColor(red);
    //delay(1000);
    //led.off();
    //delay(1000);
    //led.setColor(green);
    //delay(1000);
    //led.off();
    //delay(1000);
    //led.setColor(blue);
    //delay(1000);
    //led.off();
    //delay(1000);

    //led.fadeOnce(red, black, 30000);
    setColor("255,0,0");
    delay(5000);
    setColor("0,0,0");
    delay(5000);

    //sendHttpRequest("post", "davidenastri.it", 8080, "/", "Ciao");
    //sendHttpRequest("get", "davidenastri.it", 8080, "/", "Ciao");

}
