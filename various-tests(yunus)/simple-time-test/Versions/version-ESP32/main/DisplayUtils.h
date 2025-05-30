// File: DisplayUtils.h
// Purpose: Declares functions for initializing and drawing on the 4.2-inch e-paper display.
//          Keeps display-related tasks separate for clarity and reusability.
// Author: Jonas Kjeldmand Jensen
// Date: May 30, 2025

#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                        LIBRARY INCLUDES                                                           ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

#include <GxEPD2_BW.h>    // Core e-paper display library for black/white displays
#include <ArduinoJson.h>  // JSON parsing for API response handling
#include "TimeUtils.h"    // Time structure and utilities for NTP synchronization

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                    DISPLAY TYPE DEFINITION                                                        ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║  Define a shorter alias for the specific 4.2" e-paper display type to improve code readability                  ║
// ║  Model: GDEY042T81 (400x300 pixel resolution, black and white)                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

typedef GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> EpaperDisplay;

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                     FUNCTION DECLARATIONS                                                         ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║  setupDisplay():        Initialize the e-paper display with proper settings for rotation and text color         ║
// ║  displayBusArrivals():  Parse JSON bus data and render a clean layout showing bus stop times and arrivals       ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

void setupDisplay(EpaperDisplay& display);
void displayBusArrivals(JsonObject arrivals, TimeData currentTime, EpaperDisplay& display);

#endif