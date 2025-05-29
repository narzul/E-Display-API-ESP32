// File: DisplayUtils.h
// Purpose: Declares functions for initializing and drawing on the 4.2-inch e-paper display.
// Keeps display-related tasks separate for clarity and reusability.

#ifndef DISPLAY_UTILS_H  // Prevent double inclusion
#define DISPLAY_UTILS_H

#include <GxEPD2_BW.h>

// Function declarations
void setupDisplay(GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>& display);
void drawBusStopDisplay(
  GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>& display,
  String stopName1583, String arrivalText1583,
  String stopName1550, String arrivalText1550,
  String currentTime
);

#endif