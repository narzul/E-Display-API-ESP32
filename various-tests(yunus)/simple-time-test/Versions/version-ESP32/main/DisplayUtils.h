// File: DisplayUtils.h
// Purpose: Declares functions for initializing and drawing on the 4.2-inch e-paper display.
// Keeps display-related tasks separate for clarity and reusability.

#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include <GxEPD2_BW.h>
#include <ArduinoJson.h>
#include "TimeUtils.h"

void setupDisplay(GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>& display);
void displayBusArrivals(JsonObject arrivals, TimeData currentTime, GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>& display);

#endif