/**
* Baby Sleep Training Lamp
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
String method; // "get" and "post" are currently implemented
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



bool fadeColor(String command)
{
    // Getting index of all the commas in the command
    int firstCommaIndex = command.indexOf(',');
    int secondCommaIndex = command.indexOf(',', firstCommaIndex+1);
    int thirdCommaIndex = command.indexOf(',', secondCommaIndex+1);
    int fourthCommaIndex = command.indexOf(',', thirdCommaIndex+1);
    int fifthCommaIndex = command.indexOf(',', fourthCommaIndex+1);
    
    // Using that indexes to split the command subunits
    String intensityRedStart = command.substring(0, firstCommaIndex);
    String intensityGreenStart = command.substring(firstCommaIndex+1, secondCommaIndex);
    String intensityBlueStart = command.substring(secondCommaIndex+1, thirdCommaIndex);
    String intensityRedEnd = command.substring(thirdCommaIndex+1, fourthCommaIndex);
    String intensityGreenEnd = command.substring(fourthCommaIndex+1, fifthCommaIndex);
    String intensityBlueEnd = command.substring(fifthCommaIndex+1);

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
    
    // Start color variable
    Color start(intensityRedStart.toInt(), intensityGreenStart.toInt(), intensityBlueStart.toInt());
    
    // End color variable
    Color end(intensityRedEnd.toInt(), intensityGreenEnd.toInt(), intensityBlueEnd.toInt());
    
    // Fade one from Start color to End color
    led.fadeOnce(start, end, 30000);
    
    return true;
}


void setup() {
    // Starting serial port
    Serial.begin(9600);
    // Publish functions on the Particle Cloud
    Particle.function("fadeColor", fadeColor);
    Particle.function("setColor", setColor);
    // When the lamp is turned on the LED is off
    led.off();
}


void loop() {
    
    // Pulse red from 0% brightness to 100% brightness every 5 seconds
    //led.fade(red.withBrightness(10), red, 5000);
    
    // Example http calls
    //sendHttpRequest("post", "davidenastri.it", 8080, "/", "Ciao");
    //sendHttpRequest("get", "davidenastri.it", 8080, "/", "Ciao");

}
