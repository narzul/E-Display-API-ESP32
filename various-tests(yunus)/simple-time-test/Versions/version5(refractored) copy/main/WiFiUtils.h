// File: WiFiUtils.h
// Purpose: Declares functions for connecting to WiFi and keeping the connection
// stable. Keeps WiFi tasks separate for clarity and easy updates.

#ifndef WIFI_UTILS_H  // Prevent double inclusion
#define WIFI_UTILS_H

// Function declarations
void setupWiFi();
void maintainWiFi();

#endif