// File: TimeUtils.cpp
// Purpose: Contains the logic for getting the current time from an NTP server
// and displaying it. Separates time tasks from the main program.

#include "time.h"
#include "WiFiUtils.h"  // Needed for WiFi maintenance
#include "TimeUtils.h"

// NTP server settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;  // UTC+2 for CEST
const int   daylightOffset_sec = 0;

// Set up the NTP connection to get accurate time
void setupNTP() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(1000);  // Give time to sync
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // Uncomment the following line to enable Serial debugging for NTP sync failure
    // Serial.println("Initial NTP sync failed, retrying in loop...");
  }
}

// Get the current time from NTP with error handling
TimeData getCurrentTimeFromNTP() {
  TimeData timeData = {-1, -1, -1};
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    // Uncomment the following block to enable Serial debugging for time sync issues
    /*
    Serial.println("Failed to obtain time, attempting to resync...");
    */
    maintainWiFi();  // Ensure WiFi is connected
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(1000);  // Wait a second for sync
    if (!getLocalTime(&timeinfo)) {
      // Uncomment the following line to enable Serial debugging for persistent sync failure
      // Serial.println("Still failed to obtain time.");
      return timeData;  // Return invalid time if sync fails
    }
  }

  timeData.hours = timeinfo.tm_hour;
  timeData.minutes = timeinfo.tm_min;
  timeData.seconds = timeinfo.tm_sec;

  return timeData;
}

// Print time in a clear HH:MM:SS format
void printTime(TimeData time) {
  // Uncomment the following block to enable Serial debugging for time display
  /*
  if (time.hours < 10) Serial.print("0");
  Serial.print(time.hours);
  Serial.print(":");
  if (time.minutes < 10) Serial.print("0");
  Serial.print(time.minutes);
  Serial.print(":");
  if (time.seconds < 10) Serial.print("0");
  Serial.println(time.seconds);
  */
}