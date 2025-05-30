/* File: TimeUtils.cpp
   Purpose: Implementation of NTP time synchronization and utilities for Arduino Nano ESP32.
            Handles timezone configuration for Denmark and provides robust time handling.
   Author: Jonas Kjeldmand Jensen
   Date: May 30, 2025
*/

#include <Arduino.h>
#include <time.h>
#include "TimeUtils.h"

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                      NTP SERVER CONFIGURATION                                                     ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║                                                                                                                   ║
// ║  NTP (Network Time Protocol) Settings for Denmark (Central European Time Zone)                                  ║
// ║                                                                                                                   ║
// ║  • pool.ntp.org        - Global pool of time servers (automatically finds closest)                              ║
// ║  • GMT Offset: +1 hour - Central European Time (CET) is UTC+1                                                   ║
// ║  • DST Offset: +1 hour - Daylight Saving Time adds another hour (UTC+2 in summer)                              ║
// ║                                                                                                                   ║
// ║  Note: ESP32's configTime() automatically handles DST transitions                                                ║
// ║                                                                                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;      // GMT+1 for Central European Time (3600 seconds = 1 hour)
const int daylightOffset_sec = 3600;  // DST adjustment (3600 seconds = 1 hour)

void setupNTP() {
  Serial.println("   📡 Configuring NTP client for Danish timezone (CET/CEST)...");
  Serial.println("   🌐 NTP Server: pool.ntp.org");
  Serial.println("   🕐 Timezone: UTC+1 (CET) / UTC+2 (CEST with DST)");
  
  // Configure the ESP32's internal time system
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Wait for initial time synchronization
  Serial.print("   ⏳ Waiting for time sync");
  int attempts = 0;
  while (attempts < 10) {  // Max 10 attempts (5 seconds)
    delay(500);
    Serial.print(".");
    attempts++;
    
    // Check if we got a valid time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      Serial.println();
      Serial.println("   ✅ NTP synchronization successful!");
      return;
    }
  }
  
  Serial.println();
  Serial.println("   ⚠️  NTP sync timeout - will retry in background");
}

TimeData getCurrentTimeFromNTP() {
  struct tm timeinfo;
  TimeData currentTime = {-1, -1, -1};  // Initialize with invalid time markers
  
  // Attempt to get local time from the ESP32's synchronized clock
  if (getLocalTime(&timeinfo)) {
    // Successfully got time - extract hours, minutes, seconds
    currentTime.hours = timeinfo.tm_hour;    // 0-23
    currentTime.minutes = timeinfo.tm_min;   // 0-59  
    currentTime.seconds = timeinfo.tm_sec;   // 0-59
  } else {
    // Time not available - could be initial boot or network issue
    Serial.println("⚠️  Warning: Unable to get current time from NTP");
  }
  
  return currentTime;
}

void printTime(TimeData time) {
  if (!isTimeValid(time)) {
    Serial.println("Time: --:--:-- (Invalid/Not Synced)");
    return;
  }
  
  // Format and print time with leading zeros for consistency
  // Output format: "HH:MM:SS"
  Serial.print("Time: ");
  
  // Hours (00-23)
  if (time.hours < 10) Serial.print("0");
  Serial.print(time.hours);
  Serial.print(":");
  
  // Minutes (00-59)
  if (time.minutes < 10) Serial.print("0");
  Serial.print(time.minutes);
  Serial.print(":");
  
  // Seconds (00-59)
  if (time.seconds < 10) Serial.print("0");
  Serial.println(time.seconds);
}

bool isTimeValid(TimeData time) {
  // Check if the time data contains valid values
  // Invalid time is marked with -1 values
  return (time.hours >= 0 && time.hours <= 23 &&
          time.minutes >= 0 && time.minutes <= 59 &&
          time.seconds >= 0 && time.seconds <= 59);
}