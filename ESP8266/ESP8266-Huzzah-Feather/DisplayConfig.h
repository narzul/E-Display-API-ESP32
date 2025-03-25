/*
* Made by Jonas Kjeldmnd Jensen (Yunus), April 2025 
* 
* This code is setup to work with the Adafruit Feather HUZZAH ESP8266's pinout
* https://learn.adafruit.com/adafruit-feather-huzzah-esp8266/overview#uts
*
*/

#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

// Initialize the display for the 4.2" ePaper module
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(
  /*CS=5*/   5,    // Chip Select pin
  /*DC=*/    0,    // Data/Command pin
  /*RST=*/   2,    // Reset pin
  /*BUSY=*/  15    // Busy pin
  // SCL (Serial Clock) is connected to GPIO 18
  // SDA (Serial Data / MOSI) is connected to GPIO 23
));

#endif