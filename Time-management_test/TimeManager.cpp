/*
* TimeManager.cpp
* Made by Jonas Kjeldmnd Jensen (Yunus), September 2024
* Extended with smart time management functionality
*
* Complete implementation of time management functions with efficient
* NTP syncing and comprehensive time calculations
*/

#include "TimeManager.h"

// =========================
// CONFIGURATION CONSTANTS
// =========================

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;        // GMT+1 for Copenhagen (adjust as needed)
const int daylightOffset_sec = 3600;    // Daylight saving time offset

// Timing intervals (in milliseconds)
const unsigned long NTP_SYNC_INTERVAL = 12UL * 60UL * 60UL * 1000UL;      // 12 hours
const unsigned long WEATHER_UPDATE_INTERVAL = 10UL * 60UL * 1000UL;       // 10 minutes  
const unsigned long DISPLAY_UPDATE_INTERVAL = 30UL * 1000UL;              // 30 seconds

// =========================
// GLOBAL VARIABLES
// =========================

TimeInfo storedTime;
WeatherData currentWeather;
bool timeInitialized = false;
unsigned long lastNtpSync = 0;
unsigned long lastWeatherUpdate = 0;
unsigned long lastDisplayUpdate = 0;

// =========================
// CORE TIME FUNCTIONS
// =========================

bool initializeTime() {
  Serial.println("🕐 Fetching time from NTP server...");
  
  // Configure time with NTP server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Wait for time to be set (up to 10 seconds)
  int attempts = 0;
  while (!isTimeValid() && attempts < 10) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (isTimeValid()) {
    timeInitialized = true;
    lastNtpSync = millis();
    TimeInfo currentTime = getCurrentTime();
    Serial.println("✓ Time synchronized: " + formatTime(currentTime));
    return true;
  } else {
    Serial.println("✗ Failed to synchronize time");
    return false;
  }
}

TimeInfo getCurrentTime() {
  TimeInfo timeInfo = {0}; // Initialize all fields to 0
  
  if (!timeInitialized) {
    Serial.println("⚠️  Warning: Time not initialized!");
    return timeInfo;
  }
  
  struct tm timeStruct;
  time_t now;
  time(&now);
  localtime_r(&now, &timeStruct);
  
  timeInfo.year = timeStruct.tm_year + 1900;
  timeInfo.month = timeStruct.tm_mon + 1;
  timeInfo.day = timeStruct.tm_mday;
  timeInfo.hour = timeStruct.tm_hour;
  timeInfo.minute = timeStruct.tm_min;
  timeInfo.second = timeStruct.tm_sec;
  timeInfo.timestamp = now;
  
  return timeInfo;
}

TimeInfo getStoredTime() {
  return storedTime;
}

void storeCurrentTime() {
  storedTime = getCurrentTime();
  Serial.println("📌 Reference time stored: " + formatTime(storedTime));
}

bool isTimeValid() {
  struct tm timeStruct;
  time_t now;
  time(&now);
  localtime_r(&now, &timeStruct);
  
  // Check if year is reasonable (after 2020)
  return (timeStruct.tm_year + 1900) > 2020;
}

// =========================
// TIME CALCULATION FUNCTIONS
// =========================

int getTimeDifferenceMinutes(TimeInfo time1, TimeInfo time2) {
  // Calculate difference in seconds, then convert to minutes
  long diffSeconds = time2.timestamp - time1.timestamp;
  return diffSeconds / 60;
}

int getTimeDifferenceSeconds(TimeInfo time1, TimeInfo time2) {
  return time2.timestamp - time1.timestamp;
}

// =========================
// TIME FORMATTING FUNCTIONS
// =========================

String formatTime(TimeInfo timeInfo) {
  if (timeInfo.timestamp == 0) {
    return "Invalid time";
  }
  
  char buffer[25];
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", 
          timeInfo.year, timeInfo.month, timeInfo.day,
          timeInfo.hour, timeInfo.minute, timeInfo.second);
  return String(buffer);
}

String formatTimeShort(TimeInfo timeInfo) {
  if (timeInfo.timestamp == 0) {
    return "Invalid";
  }
  
  char buffer[10];
  sprintf(buffer, "%02d:%02d:%02d", timeInfo.hour, timeInfo.minute, timeInfo.second);
  return String(buffer);
}

String formatTimeDifference(int minutes) {
  String result = "";
  
  if (minutes >= 0) {
    result += "+";
  } else {
    result += "-";
    minutes = abs(minutes);  // Make positive for formatting
  }
  
  if (minutes >= 60) {
    int hours = minutes / 60;
    int remainingMinutes = minutes % 60;
    result += String(hours) + "h";
    if (remainingMinutes > 0) {
      result += " " + String(remainingMinutes) + "m";
    }
  } else {
    result += String(minutes) + "m";
  }
  
  return result;
}

String formatDate(TimeInfo timeInfo) {
  if (timeInfo.timestamp == 0) {
    return "Invalid date";
  }
  
  char buffer[15];
  sprintf(buffer, "%04d-%02d-%02d", timeInfo.year, timeInfo.month, timeInfo.day);
  return String(buffer);
}

// =========================
// TIME MANAGEMENT FUNCTIONS
// =========================

void checkForNtpSync() {
  // Re-sync every 12 hours automatically
  if (millis() - lastNtpSync > NTP_SYNC_INTERVAL) {
    Serial.println("🔄 12 hours passed, re-syncing time...");
    if (initializeTime()) {
      Serial.println("✓ Time re-synced successfully");
    } else {
      Serial.println("✗ Time re-sync failed, will retry later");
    }
  }
}

bool shouldUpdateWeather() {
  return (millis() - lastWeatherUpdate > WEATHER_UPDATE_INTERVAL);
}

bool shouldUpdateDisplay() {
  return (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL);
}

// =========================
// UTILITY FUNCTIONS
// =========================

void printTimeStatus() {
  TimeInfo currentTime = getCurrentTime();
  int uptimeMinutes = getTimeDifferenceMinutes(storedTime, currentTime);
  
  Serial.println("\n--- Time Status ---");
  Serial.println("Current time: " + formatTime(currentTime));
  Serial.println("Stored time:  " + formatTime(storedTime));
  Serial.println("Uptime:       " + formatTimeDifference(uptimeMinutes));
  Serial.println("Time valid:   " + String(timeInitialized ? "Yes" : "No"));
  
  if (currentWeather.valid) {
    int weatherAge = getTimeDifferenceMinutes(currentWeather.fetchTime, currentTime);
    Serial.println("Weather age:  " + formatTimeDifference(weatherAge));
  }
  Serial.println("------------------");
}

unsigned long getUptimeSeconds() {
  return millis() / 1000;
}