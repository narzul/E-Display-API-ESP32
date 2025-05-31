// File: APIUtils.h
// Purpose: Declares tools for fetching bus arrival data from the Rejseplanen API
// and preparing it for display. Keeps API logic separate for easy reading.

#ifndef API_UTILS_H  // Prevent double inclusion
#define API_UTILS_H

#include "TimeUtils.h"  // Need TimeData structure
#include <GxEPD2_BW.h>  // Need display type

// Function declarations
void setupAPIFilter();
void checkBusArrivals(TimeData currentTime, GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>& display);

#endif