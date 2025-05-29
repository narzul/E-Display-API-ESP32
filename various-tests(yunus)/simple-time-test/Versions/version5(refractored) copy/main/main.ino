/* File: main.ino
   Purpose: Main file for the bus stop sign project with a 4.2-inch e-paper display.
   Sets up the system, including WiFi, NTP, API, and the display, and runs the loop
   to check time and bus arrivals every minute. Keeps the core logic simple by calling
   functions from other files.
*/

// Include necessary libraries for the display
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

// Include our module headers
#include "TimeUtils.h"
#include "WiFiUtils.h"
#include "APIUtils.h"
#include "DisplayUtils.h"

// ESP8266 Feather HUZZAH Pin Connections for 4.2" EPD Module:
// EPD Module Pin -> Feather HUZZAH Pin (Function)
// ----------------------------------------------
// BUSY  -> GPIO #16 (BUSY)
// RESET -> GPIO #5  (RES/RST)
// DC    -> GPIO #4  (DC)
// CS    -> GPIO #15 (CS/SS)
// SCL   -> GPIO #14 (SCK/SCL)
// SDA   -> GPIO #13 (MOSI/SDA)
// GND   -> GND      (Ground)
// VCC   -> 3V       (3.3V Power)

// Define the e-paper display HERE in the main sketch
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
  GxEPD2_420_GDEY042T81(/*CS=*/ 15, /*DC=*/ 4, /*RES=*/ 5, /*BUSY=*/ 16)
); // 400x300, SSD1683

void setup() {
  // Uncomment the following line to enable Serial communication for debugging
  // Serial.begin(115200);
  delay(1000);

  // Initialize the display
  setupDisplay(display);

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

    // Uncomment the following block to enable Serial debugging for current time
    /*
    Serial.println("=== Time Info ===");
    printTime(currentTime);
    */

    // Fetch and process bus arrivals, passing the display to draw on it
    checkBusArrivals(currentTime, display);

    // Uncomment the following line to enable Serial debugging separator
    // Serial.println("=================");
  }

  delay(100);  // Check 10 times per second to catch minute changes
}