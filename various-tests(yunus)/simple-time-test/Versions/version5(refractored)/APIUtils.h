// File: APIUtils.h
// Purpose: Declares tools for fetching bus arrival data from the Rejseplanen API
// and showing it. Keeps API and bus logic separate for easy reading.

#ifndef API_UTILS_H  // Prevent double inclusion
#define API_UTILS_H

#include "TimeUtils.h"  // Need TimeData structure

// Function declarations
void setupAPIFilter();
void checkBusArrivals(TimeData currentTime);

#endif