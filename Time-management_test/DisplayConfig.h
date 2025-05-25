/*
* DisplayConfig.h
* Made by Jonas Kjeldmnd Jensen (Yunus), September 2024
* 
* This code is setup to work with the Sparkfun ESP32 Thing's pinout
* https://learn.sparkfun.com/tutorials/esp32-thing-hookup-guide/all
*/

#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <GxEPD2_BW.h>

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