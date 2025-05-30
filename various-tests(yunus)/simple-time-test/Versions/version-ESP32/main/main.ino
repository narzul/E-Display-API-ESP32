/* File: main.ino
   Purpose: Main entry point for the bus stop sign project using an Arduino Nano ESP32.
            Integrates a 4.2-inch e-paper display to show bus arrival times fetched from
            the Rejseplanen API, with time synchronized via NTP over WiFi.
   Author: [Your Name]
   Date: [Current Date]
*/

// Include libraries for the e-paper display
#include <GxEPD2_BW.h>              // Library for controlling e-paper displays
#include <Fonts/FreeMonoBold9pt7b.h> // Small bold font for text
#include <Fonts/FreeMonoBold12pt7b.h> // Medium bold font for text
#include <Fonts/FreeMonoBold24pt7b.h> // Large bold font for time or headers

// Include custom module headers
#include "TimeUtils.h"   // For NTP time synchronization
#include "WiFiUtils.h"   // For WiFi connection management
#include "APIUtils.h"    // For fetching and processing API data
#include "DisplayUtils.h" // For managing the e-paper display

// Define the e-paper display object for a 4.2-inch display (400x300 resolution)
// Compatible with Arduino Nano ESP32's pinout and ESP32-S3 chipset
// SPI pins: SCK=D13 (GPIO18), MOSI=D11 (GPIO23), MISO=D12 (GPIO19, not used here)
// Custom pins: CS=D10 (GPIO10), DC=D9 (GPIO9), RES=D8 (GPIO8), BUSY=D7 (GPIO7)
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
  GxEPD2_420_GDEY042T81(/*CS=*/ 10, /*DC=*/ 9, /*RES=*/ 8, /*BUSY=*/ 7)
);

// Setup function: Runs once when the Arduino starts
void setup() {
  // Initialize serial communication for debugging (optional)
  // Serial.begin(115200);  // Uncomment to enable Serial output
  delay(1000);  // Brief delay to allow hardware to stabilize

  // Initialize the e-paper display
  setupDisplay(display);

  // Connect to WiFi network
  setupWiFi();

  // Synchronize time with NTP server
  setupNTP();

  // Configure API filters or settings (if needed)
  setupAPIFilter();
}

// Loop function: Runs continuously after setup
void loop() {
  static int lastMinute = -1;  // Tracks the last minute processed to avoid duplicate updates
  TimeData currentTime = getCurrentTimeFromNTP();  // Fetch current time from NTP

  // Check if time is valid and the minute has changed
  if (currentTime.hours != -1 && currentTime.minutes != lastMinute) {
    lastMinute = currentTime.minutes;  // Update the last processed minute

    // Optional: Debug time output via Serial (uncomment to use)
    /*
    Serial.println("=== Time Info ===");
    printTime(currentTime);
    */

    // Fetch bus arrivals and update the display
    checkBusArrivals(currentTime, display);

    // Optional: Debug separator via Serial (uncomment to use)
    // Serial.println("=================");
  }

  delay(100);  // Short delay to check time 10 times per second
}