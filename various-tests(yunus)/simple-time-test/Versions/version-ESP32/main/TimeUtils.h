/* File: TimeUtils.h
   Purpose: Header file for NTP time synchronization and time utilities for Arduino Nano ESP32.
            Provides clean interface for getting current time and formatting time data.
   Author: Jonas Kjeldmand Jensen
   Date: May 30, 2025
*/

#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <Arduino.h>

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                       TIME DATA STRUCTURE                                                         ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║                                                                                                                   ║
// ║  Simple structure to hold time information in a clean, easy-to-use format                                        ║
// ║  Values of -1 indicate invalid/unsynced time                                                                      ║
// ║                                                                                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

struct TimeData {
  int hours;    // 0-23 (24-hour format)
  int minutes;  // 0-59
  int seconds;  // 0-59
};

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                     FUNCTION DECLARATIONS                                                         ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║                                                                                                                   ║
// ║  setupNTP()               - Initialize NTP time synchronization with Danish timezone                            ║
// ║  getCurrentTimeFromNTP()  - Get current time from NTP server                                                     ║
// ║  printTime()              - Print formatted time to Serial monitor                                               ║
// ║  isTimeValid()            - Check if TimeData contains valid time                                                 ║
// ║                                                                                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

void setupNTP();
TimeData getCurrentTimeFromNTP();
void printTime(TimeData time);
bool isTimeValid(TimeData time);

#endif  // TIME_UTILS_H