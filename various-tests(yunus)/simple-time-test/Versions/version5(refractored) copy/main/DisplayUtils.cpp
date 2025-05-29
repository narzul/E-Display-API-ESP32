// File: DisplayUtils.cpp
// Purpose: Contains the logic for initializing the 4.2-inch e-paper display and
// drawing bus stop arrival information in a clean, professional layout.

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include "DisplayUtils.h"

// Initialize the e-paper display
void setupDisplay(GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>& display) {
  display.init(115200, true, 50, false); // Serial baud, initial reset, reset duration, no pull-up
  display.setRotation(4); // 90 degrees counter-clockwise for landscape (400x300 pixels)
  display.setTextColor(GxEPD_BLACK); // Set default text color to black
}

// Draw the bus stop information on the display
void drawBusStopDisplay(
  GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>& display,
  String stopName1583, String arrivalText1583,
  String stopName1550, String arrivalText1550,
  String currentTime
) {
  // Mock layout of the display (commented for reference)
  /*
  +======================================+
  |          Bus Stop Times              |
  |                                      |
  |  Gammel Kongevej (Alhambravej)       |
  |  +6 min (14:35) (RT)                 |
  |                                      |
  |  Gammel Kongevej (H.C. Ørsteds Vej)  |
  |  Ingen busser indenfor næste time    |
  |                                      |
  |  Current Time: 14:29                 |
  +======================================+
  */

  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE); // Clear to white background

    // Draw a simple border
    display.drawRect(5, 5, display.width() - 10, display.height() - 10, GxEPD_BLACK);

    // Variables for text positioning
    int16_t tbx, tby; uint16_t tbw, tbh;
    uint16_t x, y;

    // Draw header
    display.setFont(&FreeMonoBold24pt7b);
    String title = "Bus Stop Times";
    display.getTextBounds(title.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2 - tbx; // Center horizontally
    y = 30; // Start near the top
    display.setCursor(x, y);
    display.print(title.c_str());

    // Draw stop 1583
    y += 50; // Move down
    display.setFont(&FreeMonoBold12pt7b);
    display.getTextBounds(stopName1583.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2 - tbx;
    display.setCursor(x, y);
    display.print(stopName1583.c_str());

    y += 25; // Move down for arrival
    display.setFont(&FreeMonoBold9pt7b);
    display.getTextBounds(arrivalText1583.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2 - tbx;
    display.setCursor(x, y);
    display.print(arrivalText1583.c_str());

    // Draw stop 1550
    y += 50; // Move down
    display.setFont(&FreeMonoBold12pt7b);
    display.getTextBounds(stopName1550.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2 - tbx;
    display.setCursor(x, y);
    display.print(stopName1550.c_str());

    y += 25; // Move down for arrival
    display.setFont(&FreeMonoBold9pt7b);
    display.getTextBounds(arrivalText1550.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2 - tbx;
    display.setCursor(x, y);
    display.print(arrivalText1550.c_str());

    // Draw current time
    y += 50; // Move down
    display.setFont(&FreeMonoBold9pt7b);
    String timeText = "Current Time: " + currentTime;
    display.getTextBounds(timeText.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2 - tbx;
    display.setCursor(x, y);
    display.print(timeText.c_str());
  } while (display.nextPage());
}