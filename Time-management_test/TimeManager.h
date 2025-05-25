/*
* TimeManager.h
* Made by Jonas Kjeldmnd Jensen (Yunus), September 2024
* Extended with smart time management functionality
*
* This header provides comprehensive time fetching and management capabilities
* for the ESP32 weather display project with efficient NTP syncing
*/

#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

// =========================
// NTP CONFIGURATION
// =========================

// NTP server configuration
extern const char* ntpServer;
extern const long gmtOffset_sec;        // GMT offset in seconds (3600 = GMT+1)
extern const int daylightOffset_sec;    // Daylight saving time offset

// Timing intervals
extern const unsigned long NTP_SYNC_INTERVAL;      // How often to re-sync NTP (12 hours)
extern const unsigned long WEATHER_UPDATE_INTERVAL; // How often to update weather (10 minutes)
extern const unsigned long DISPLAY_UPDATE_INTERVAL; // How often to update display (30 seconds)

// =========================
// TIME STRUCTURES
// =========================

// Structure to store complete time information
struct TimeInfo {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  time_t timestamp;  // Unix timestamp for easy calculations
};

// Structure to store weather data with timestamp
struct WeatherData {
  String cityName;
  float temperature;
  String description;
  bool valid;
  TimeInfo fetchTime;  // When this data was fetched
};

// =========================
// GLOBAL VARIABLES
// =========================

extern TimeInfo storedTime;           // Reference time (usually startup time)
extern WeatherData currentWeather;    // Current weather data with fetch time
extern bool timeInitialized;          // Whether NTP sync was successful
extern unsigned long lastNtpSync;     // Last NTP synchronization time
extern unsigned long lastWeatherUpdate; // Last weather API call
extern unsigned long lastDisplayUpdate; // Last display refresh

// =========================
// FUNCTION DECLARATIONS
// =========================

// Core time functions
bool initializeTime();
TimeInfo getCurrentTime();
TimeInfo getStoredTime();
void storeCurrentTime();
bool isTimeValid();

// Time calculation functions
int getTimeDifferenceMinutes(TimeInfo time1, TimeInfo time2);
int getTimeDifferenceSeconds(TimeInfo time1, TimeInfo time2);

// Time formatting functions
String formatTime(TimeInfo timeInfo);
String formatTimeShort(TimeInfo timeInfo);
String formatTimeDifference(int minutes);
String formatDate(TimeInfo timeInfo);

// Time management functions
void checkForNtpSync();
bool shouldUpdateWeather();
bool shouldUpdateDisplay();

// Utility functions
void printTimeStatus();
unsigned long getUptimeSeconds();

#endif