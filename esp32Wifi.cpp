#include <WiFi.h>
#include <HTTPClient.h>
#include "esp32Wifi.h"
#include "esp32Adc.h"
#include "main.h"

// WiFi credentials
const char* ssid = "DurgaS24+"; 
const char* password = "12312344"; 
// ThingSpeak settings
const char* apiKey = "KU80V0303ZTPF9S4"; 
const char* baseURL = "http://api.thingspeak.com/update"; 

void wifiInit() {
  WiFi.begin(ssid, password);
  //Serial.print("Connecting to WiFi\n");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
   // Serial.print(".");
  }
}

void reconnectWiFi() {
  WiFi.begin(ssid, password);
 // Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
}

void dataUpdateToCloud(float cell[], float packVoltage, float Ctemp, bool blance) {
  String url = String(baseURL) + "?api_key=" + apiKey +
  "&field1=" + String(cell[0]) +
  "&field2=" + String(cell[1]) +
  "&field3=" + String(cell[2]) +
  "&field4=" + String(cell[3]) +
  "&field5=" + String(packVoltage) +
  "&field6=" + String(FETSTATUS) +
  "&field7=" + String(Ctemp) +
  "&field8=" + String(blance);

  if (WiFi.status() == WL_CONNECTED) {
    //Serial.println("\nWiFi connected");
    HTTPClient http;
    http.begin(url);

    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      String payload = http.getString();
    //  Serial.println("Response code: " + String(httpResponseCode));
    //  Serial.println("Payload: " + payload);
    } else {
    //  Serial.println("Error on sending: " + String(httpResponseCode));
    //  Serial.println("No internet");
    }

    http.end();
  } else {
   // Serial.println("WiFi Disconnected");
    reconnectWiFi();
  }
}
