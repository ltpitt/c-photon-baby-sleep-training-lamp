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
Led led(D0, D1, D2);
Color red(255, 0, 0);


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

void setup() {
    Serial.begin(9600);
}

void loop() {
    // Pulse red from 0% brightness to 100% brightness every 5 seconds
    led.fade(red.withBrightness(10), red, 5000);
    
    //sendHttpRequest("post", "davidenastri.it", 8080, "/", "Ciao");
    //sendHttpRequest("get", "davidenastri.it", 8080, "/", "Ciao");

}

