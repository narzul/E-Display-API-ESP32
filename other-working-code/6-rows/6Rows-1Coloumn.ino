#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

// Initialize the display for the 4.2" ePaper module
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(
  /*CS=5*/   5,    // Chip Select pin
  /*DC=*/    0,    // Data/Command pin
  /*RST=*/   2,    // Reset pin
  /*BUSY=*/  15    // Busy pin
  // SCL (Serial Clock) is connected to GPIO 18
  // SDA (Serial Data / MOSI) is connected to GPIO 23
));

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
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    // Set the font and text color
    display.setFont(&FreeMonoBold12pt7b);
    display.setTextColor(GxEPD_BLACK);

    // Calculate the starting position for the first row
    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds("H.C. Ørstedsvej", 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = ((display.height() - (9 * tbh)) / 2) - tby;

    // Draw the table rows
    display.setCursor(x, y);
    display.print("H.C. Ørstedsvej");
    y += tbh + 5;
    display.setCursor(x, y);
    display.print("");
    y += tbh + 5;
    display.setCursor(x, y);
    display.print("");
    y += tbh + 5;
    display.setCursor(x, y);
    display.print("Gammel Kongevej");
    y += tbh + 5;
    display.setCursor(x, y);
    display.print("");
    y += tbh + 5;
    display.setCursor(x, y);
    display.print("");
    y += tbh + 5;
    display.setCursor(x, y);
    display.print("Frederiksberg Allé");
    y += tbh + 5;
    display.setCursor(x, y);
    display.print("");
    y += tbh + 5;
    display.setCursor(x, y);
    display.print("");
  }
  while (display.nextPage());
}

void loop() {
  // The main loop is empty as all the work is done in setup()
}