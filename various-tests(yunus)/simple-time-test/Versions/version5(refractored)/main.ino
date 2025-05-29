/* File: main.ino
   Purpose: Main file for the bus stop sign project. Sets up the system and runs the loop
   to check time and bus arrivals every minute. Keeps the core logic simple by calling
   functions from other files.
*/

// Include the header files for our modules
#include "TimeUtils.h"
#include "WiFiUtils.h"
#include "APIUtils.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Set up WiFi connection
  setupWiFi();

  // Set up NTP for time synchronization
  setupNTP();

  // Set up the API filter for bus arrival data
  setupAPIFilter();
}

void loop() {
  static int lastMinute = -1;
  TimeData currentTime = getCurrentTimeFromNTP();

  // Process only if time is valid and the minute has changed
  if (currentTime.hours != -1 && currentTime.minutes != lastMinute) {
    lastMinute = currentTime.minutes;

    // Display time information
    Serial.println("=== Time Info ===");
    printTime(currentTime);

    // Fetch and process bus arrivals
    checkBusArrivals(currentTime);

    Serial.println("=================");
  }

  delay(100);  // Check 10 times per second to catch minute changes
}