#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "credentials.h"
#include "api_config.h"
#include "api_handler.h"

// Initialize the display for the 4.2" ePaper module
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(
  /*CS=5*/   5,    // Chip Select pin
  /*DC=*/    0,    // Data/Command pin
  /*RST=*/   2,    // Reset pin
  /*BUSY=*/  15    // Busy pin
  // SCL (Serial Clock) is connected to GPIO 18
  // SDA (Serial Data / MOSI) is connected to GPIO 23
// Note: SCL (GPIO 18) and SDA (GPIO 23) are automatically used by the ESP32's SPI interface
));


APIHandler apiHandler;

void setup()
{
  Serial.begin(115200);
  
  // Initialize WiFi
  apiHandler.begin(ssid, password);

  // Initialize the display
  display.init(115200, true, 50, false);
  
  // Fetch and display data
  displayAPIData();
  
  // Put the display in low power mode
  display.hibernate();
}

void displayAPIData()
{
  String jsonData = apiHandler.fetchData(API_URL, API_HEADER_NAME, API_HEADER_VALUE);
  String apiData = apiHandler.parseJSON(jsonData, API_JSON_KEY);
  
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(10, 30);
    display.println("API Data:");
    
    // Print the data, word-wrapping as necessary
    int16_t cursorY = 60;
    int16_t cursorX = 10;
    String line;
    int16_t x1, y1;
    uint16_t w, h;

    for (int i = 0; i < apiData.length(); i++) {
      line += apiData[i];
      display.getTextBounds(line.c_str(), cursorX, cursorY, &x1, &y1, &w, &h);
      
      if (w > display.width() - 20 || apiData[i] == '\n') {
        // Remove last character if it's not a newline
        if (apiData[i] != '\n') {
          line.remove(line.length() - 1);
          i--;
        }
        
        display.setCursor(cursorX, cursorY);
        display.print(line);
        cursorY += h + 5;
        line = "";
        
        if (cursorY > display.height() - h) {
          break;  // Stop if we've reached the bottom of the screen
        }
      }
    }
    
    // Print any remaining text
    if (line.length() > 0) {
      display.setCursor(cursorX, cursorY);
      display.print(line);
    }
  }
  while (display.nextPage());
}

void loop() {
  // The main loop is empty as all the work is done in setup()
}