/*
*
* Made by Jonas Kjeldmnd Jensen (Yunus), September 2024
*
*/

#include <WiFi.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

// Load in other header files from the project folder
#include "DisplayConfig.h"

void setup()
{
  Serial.begin(115200);
  delay(100);

  // Initialize the display
  display.init(115200, true, 50, false);
  display.setRotation(4); // Set the rotation to 90 degrees counter-clockwise

  // Draw the table
  drawTable();
}

void drawTable()
{
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);

  int rowHeight = 40;  // Height of each row in the table
  int startX = 50;     // Starting X position of the table
  int startY = 100;    // Starting Y position of the table

  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    // Draw the 9 rows
    for (int i = 0; i < 9; i++) {
      int y = startY + i * rowHeight;
      
      // Draw text in the first row
      if (i == 0) {
        display.setFont(&FreeMonoBold18pt7b);  // Bold font
        display.setCursor(startX, y);
        display.print("H.C. Ørstedsvej");
      }

      // Draw text in the fourth row
      if (i == 3) {
        display.setFont(&FreeMonoBold18pt7b);  // Bold font
        display.setCursor(startX, y);
        display.print("Gammel Kongevej");
      }

      // Draw text in the seventh row
      if (i == 6) {
        display.setFont(&FreeMonoBold18pt7b);  // Bold font
        display.setCursor(startX, y);
        display.print("Frederiksberg Allé");
      }

      // Draw the line for each row (optional, to visualize rows)
      display.drawLine(startX - 10, y - rowHeight / 2, display.width() - startX + 10, y - rowHeight / 2, GxEPD_BLACK);
    }
  }
  while (display.nextPage());
}

void loop() {
  // The main loop is empty as all the work is done in setup()
}
