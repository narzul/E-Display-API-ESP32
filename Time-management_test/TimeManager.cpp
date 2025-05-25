/*
* TimeManager.cpp
* Made by Jonas Kjeldmnd Jensen (Yunus), September 2024
* Extended for time management functionality
*
* Implementation of time management functions
*/

#include "TimeManager.h"

// Global variables
TimeInfo storedTime;
bool timeInitialized = false;

bool initializeTime() {
  Serial.println("Initializing time from NTP server...");
  
  // Configure time with NTP server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Wait for time to be set (up to 10 seconds)
  int attempts = 0;
  while (!isTimeValid() && attempts < 10) {
    delay(1000);
    attempts++;
    Serial.print(".");
  }
  Serial.println();
  
  if (isTimeValid()) {
    timeInitialized = true;
    Serial.println("Time synchronized successfully!");
    TimeInfo currentTime = getCurrentTime();
    Serial.println("Current time: " + formatTime(currentTime));
    return true;
  } else {
    Serial.println("Failed to synchronize time");
    return false;
  }
}

TimeInfo getCurrentTime() {
  TimeInfo timeInfo;
  
  if (!timeInitialized) {
    Serial.println("Warning: Time not initialized!");
    // Return empty/invalid time
    timeInfo.year = 0;
    timeInfo.month = 0;
    timeInfo.day = 0;
    timeInfo.hour = 0;
    timeInfo.minute = 0;
    timeInfo.second = 0;
    timeInfo.timestamp = 0;
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
  Serial.println("Time stored: " + formatTime(storedTime));
}

int getTimeDifferenceMinutes(TimeInfo time1, TimeInfo time2) {
  // Calculate difference in seconds, then convert to minutes
  long diffSeconds = time2.timestamp - time1.timestamp;
  return diffSeconds / 60;  // Convert to minutes
}

String formatTime(TimeInfo timeInfo) {
  if (timeInfo.timestamp == 0) {
    return "Invalid time";
  }
  
  String formattedTime = "";
  formattedTime += String(timeInfo.year) + "-";
  formattedTime += (timeInfo.month < 10 ? "0" : "") + String(timeInfo.month) + "-";
  formattedTime += (timeInfo.day < 10 ? "0" : "") + String(timeInfo.day) + " ";
  formattedTime += (timeInfo.hour < 10 ? "0" : "") + String(timeInfo.hour) + ":";
  formattedTime += (timeInfo.minute < 10 ? "0" : "") + String(timeInfo.minute) + ":";
  formattedTime += (timeInfo.second < 10 ? "0" : "") + String(timeInfo.second);
  
  return formattedTime;
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
    result += String(hours) + "h " + String(remainingMinutes) + "m";
  } else {
    result += String(minutes) + " minutes";
  }
  
  return result;
}

bool isTimeValid() {
  struct tm timeStruct;
  time_t now;
  time(&now);
  localtime_r(&now, &timeStruct);
  
  // Check if year is reasonable (after 2020)
  return (timeStruct.tm_year + 1900) > 2020;
}