// File: APIUtils.h
// Purpose: Declares tools for fetching and processing bus arrival data from the API for Arduino Nano ESP32.

#ifndef API_UTILS_H
#define API_UTILS_H

#include <GxEPD2_BW.h>
#include "TimeUtils.h"

void setupAPIFilter();
void checkBusArrivals(TimeData currentTime, GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>& display);

#endif