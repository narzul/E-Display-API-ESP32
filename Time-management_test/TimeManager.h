/*
* TimeManager.h
* Made by Jonas Kjeldmnd Jensen (Yunus), September 2024
* Extended for time management functionality
*
* This header provides time fetching and management capabilities
* for the ESP32 weather display project
*/

#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

// NTP server configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;        // GMT+1 for Copenhagen (adjust as needed)
const int daylightOffset_sec = 3600;    // Daylight saving time offset

// Structure to store time information
struct TimeInfo {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  time_t timestamp;  // Unix timestamp for easy calculations
};

// Function declarations
bool initializeTime();
TimeInfo getCurrentTime();
TimeInfo getStoredTime();
void storeCurrentTime();
int getTimeDifferenceMinutes(TimeInfo time1, TimeInfo time2);
String formatTime(TimeInfo timeInfo);
String formatTimeDifference(int minutes);
bool isTimeValid();

// Global variables
extern TimeInfo storedTime;
extern bool timeInitialized;

#endif