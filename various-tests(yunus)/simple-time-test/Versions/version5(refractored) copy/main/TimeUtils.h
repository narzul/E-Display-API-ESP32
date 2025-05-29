// File: TimeUtils.h
// Purpose: Declares tools for getting and showing the current time using NTP.
// This keeps time-related tasks separate for easy understanding and reuse.

#ifndef TIME_UTILS_H  // Prevent double inclusion
#define TIME_UTILS_H

// Structure to hold time data (hours, minutes, seconds)
struct TimeData {
  int hours;
  int minutes;
  int seconds;
};

// Function declarations
void setupNTP();
TimeData getCurrentTimeFromNTP();
void printTime(TimeData time);

#endif