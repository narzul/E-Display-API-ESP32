// File: TimeUtils.cpp
// Purpose: Implements tools for getting and showing the current time using NTP for Arduino Nano ESP32.

#include <Arduino.h>
#include <time.h>
#include "TimeUtils.h"

// NTP server settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;  // GMT+1 for Denmark
const int daylightOffset_sec = 3600;  // DST adjustment

void setupNTP() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // Wait for time to sync (simple delay, could be improved)
  delay(2000);
}

TimeData getCurrentTimeFromNTP() {
  struct tm timeinfo;
  TimeData currentTime = {-1, -1, -1};  // Default to invalid time

  if (getLocalTime(&timeinfo)) {
    currentTime.hours = timeinfo.tm_hour;
    currentTime.minutes = timeinfo.tm_min;
    currentTime.seconds = timeinfo.tm_sec;
  }

  return currentTime;
}

void printTime(TimeData time) {
  Serial.print("Time: ");
  if (time.hours < 10) Serial.print("0");
  Serial.print(time.hours);
  Serial.print(":");
  if (time.minutes < 10) Serial.print("0");
  Serial.print(time.minutes);
  Serial.print(":");
  if (time.seconds < 10) Serial.print("0");
  Serial.println(time.seconds);
}