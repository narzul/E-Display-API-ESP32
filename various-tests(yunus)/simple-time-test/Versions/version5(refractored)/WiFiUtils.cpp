// File: WiFiUtils.cpp
// Purpose: Contains the logic for connecting to WiFi and ensuring the connection
// stays active. Separates WiFi tasks from other parts of the program.

#include <WiFi.h>
#include "WiFiUtils.h"

// WiFi credentials
const char* ssid = "Labitat (free)";
const char* password = "labitatisawesome";

// Set up the WiFi connection
void setupWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Keep the WiFi connection active
void maintainWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("WiFi reconnected.");
  }
}