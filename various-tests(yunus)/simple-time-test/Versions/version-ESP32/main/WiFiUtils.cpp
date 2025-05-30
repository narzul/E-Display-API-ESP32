// File: WiFiUtils.cpp
// Purpose: Contains the logic for connecting to WiFi and ensuring the connection
// stays active. Separates WiFi tasks from other parts of the program.

#include <WiFi.h>
#include "WiFiUtils.h"

// WiFi credentials
const char* ssid = "Fibernet-60713347";
const char* password = "f5567r6k";

// Set up the WiFi connection
void setupWiFi() {
  // Uncomment the following block to enable Serial debugging for WiFi connection
  /*
  Serial.print("Connecting to WiFi");
  */
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // Uncomment the following line to show connection progress
    // Serial.print(".");
  }
  // Uncomment the following block to confirm WiFi connection
  /*
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  */
}

// Keep the WiFi connection active
void maintainWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    // Uncomment the following block to enable Serial debugging for WiFi reconnection
    /*
    Serial.println("WiFi disconnected, attempting to reconnect...");
    */
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      // Uncomment the following line to show reconnection progress
      // Serial.print(".");
    }
    // Uncomment the following line to confirm reconnection
    // Serial.println("WiFi reconnected.");
  }
}