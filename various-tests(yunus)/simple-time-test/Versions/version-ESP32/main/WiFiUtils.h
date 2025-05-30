// File: WiFiUtils.h
// Purpose: Declares WiFi connection tools for the Arduino Nano ESP32.
//          Handles initial connection setup and automatic reconnection management.
// Author: Jonas Kjeldmand Jensen
// Date: May 30, 2025

#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                        LIBRARY INCLUDES                                                           ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

#include <WiFi.h>  // ESP32 WiFi library for network connectivity

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                     FUNCTION DECLARATIONS                                                         ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║  setupWiFi():     Establish initial WiFi connection with credentials and connection monitoring                   ║
// ║  maintainWiFi():  Check connection status and automatically reconnect if network is lost                        ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

void setupWiFi();
void maintainWiFi();

#endif