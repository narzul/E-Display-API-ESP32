// File: DisplayUtils.cpp
// Purpose: Contains the logic for initializing the 4.2-inch e-paper display and
//          drawing bus stop arrival information in a clean, professional layout.
// Author: Jonas Kjeldmand Jensen
// Date: May 30, 2025

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                        LIBRARY INCLUDES                                                           ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

#include <Arduino.h>                   // Core Arduino functionality
#include <Fonts/FreeMonoBold9pt7b.h>   // Small bold monospace font for details
#include <Fonts/FreeMonoBold12pt7b.h>  // Medium bold monospace font for stop names
#include <Fonts/FreeMonoBold24pt7b.h>  // Large bold monospace font for headers
#include "DisplayUtils.h"              // Our display function declarations

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                    DISPLAY INITIALIZATION                                                         ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║  Initialize the 4.2-inch e-paper display with optimal settings for the bus stop sign project                    ║
// ║                                                                                                                   ║
// ║  Parameters Explained:                                                                                            ║
// ║  • 115200:     Serial communication speed (fast enough for debugging without overwhelming system)               ║
// ║  • true:       Perform initial reset on startup (ensures clean state)                                           ║
// ║  • 50:         Reset duration in milliseconds (sufficient for complete hardware reset)                          ║
// ║  • false:      Disable internal pull-up resistors (external resistors handle this)                              ║
// ║                                                                                                                   ║
// ║  Display Configuration:                                                                                           ║
// ║  • Rotation 4: 90° counter-clockwise landscape mode (400x300 becomes 300x400 effective)                        ║
// ║  • Text Color: Black text on white background for maximum contrast and readability                              ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

void setupDisplay(EpaperDisplay& display) {
  Serial.println("   📋 Display initialization parameters:");
  Serial.println("      • Serial baud: 115200");
  Serial.println("      • Initial reset: Enabled");
  Serial.println("      • Reset duration: 50ms");
  Serial.println("      • Internal pull-ups: Disabled");
  
  display.init(115200, true, 50, false);
  
  Serial.println("   🔄 Setting display orientation to landscape mode (rotation 4)");
  display.setRotation(4);  // 90 degrees counter-clockwise for landscape orientation
  
  Serial.println("   🎨 Configuring text color to black on white background");
  display.setTextColor(GxEPD_BLACK);
  
  Serial.println("   ✅ Display configuration complete - Ready for content rendering");
}

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                      BUS ARRIVAL DISPLAY                                                         ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║  Parse JSON bus arrival data and render it in a clean, readable layout on the e-paper display                   ║
// ║                                                                                                                   ║
// ║  Expected JSON Structure:                                                                                         ║
// ║  {                                                                                                                ║
// ║    "stop1583": {                                                                                                  ║
// ║      "name": "Gammel Kongevej (Alhambravej)",                                                                     ║
// ║      "arrivals": [                                                                                                ║
// ║        { "time": "14:35", "delay": 6, "realtime": true }                                                         ║
// ║      ]                                                                                                            ║
// ║    },                                                                                                             ║
// ║    "stop1550": { ... }                                                                                            ║
// ║  }                                                                                                                ║
// ║                                                                                                                   ║
// ║  Display Layout (400x300 landscape):                                                                              ║
// ║  ┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────┐ ║
// ║  │                                    Bus Stop Times                                    │ (24pt header)          │ ║
// ║  │                                                                                      │                        │ ║
// ║  │                         Gammel Kongevej (Alhambravej)                               │ (12pt stop name)       │ ║
// ║  │                              +6 min (14:35) (RT)                                    │ (9pt arrival info)     │ ║
// ║  │                                                                                      │                        │ ║
// ║  │                      Gammel Kongevej (H.C. Ørsteds Vej)                             │ (12pt stop name)       │ ║
// ║  │                           Ingen busser indenfor næste time                          │ (9pt arrival info)     │ ║
// ║  │                                                                                      │                        │ ║
// ║  │                               Current Time: 14:29                                   │ (9pt current time)     │ ║
// ║  └──────────────────────────────────────────────────────────────────────────────────────────────────────────────┘ ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

void displayBusArrivals(JsonObject arrivals, TimeData currentTime, EpaperDisplay& display) {
  Serial.println("   🎨 Starting display rendering process...");
  
  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  // DISPLAY BUFFER PREPARATION
  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  Serial.println("      • Preparing full-window display buffer");
  display.setFullWindow();
  display.firstPage();
  
  do {
    // Clear the entire display to white background
    display.fillScreen(GxEPD_WHITE);
    
    // Draw a clean border around the entire display for professional appearance
    Serial.println("      • Drawing display border");
    display.drawRect(5, 5, display.width() - 10, display.height() - 10, GxEPD_BLACK);
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // TEXT POSITIONING VARIABLES
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // These variables help with precise text centering and positioning
    int16_t tbx, tby;      // Text boundary X and Y offsets (can be negative)
    uint16_t tbw, tbh;     // Text boundary width and height
    uint16_t x, y;         // Final cursor position for drawing text
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // HEADER: "Bus Stop Times"
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    Serial.println("      • Rendering main header");
    display.setFont(&FreeMonoBold24pt7b);
    String title = "Bus Stop Times";
    
    // Calculate exact center position for the title
    display.getTextBounds(title.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2 - tbx;  // Center horizontally, adjust for text bounds
    y = 35;  // Start near the top with some padding
    
    display.setCursor(x, y);
    display.print(title.c_str());
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // BUS STOP 1583: Gammel Kongevej (Alhambravej)
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    y += 55;  // Move down from header with generous spacing
    Serial.println("      • Processing bus stop 1583 data");
    
    // Extract stop name and arrival information
    String stopName1583 = "Stop 1583";  // Default fallback
    String arrivalText1583 = "No data available";  // Default fallback
    
    if (arrivals.containsKey("stop1583")) {
      JsonObject stop1583 = arrivals["stop1583"];
      if (stop1583.containsKey("name")) {
        stopName1583 = stop1583["name"].as<String>();
      }
      if (stop1583.containsKey("arrivals") && stop1583["arrivals"].is<JsonArray>()) {
        JsonArray arrivals1583 = stop1583["arrivals"];
        if (arrivals1583.size() > 0) {
          JsonObject firstArrival = arrivals1583[0];
          arrivalText1583 = firstArrival["formatted"].as<String>();
        } else {
          arrivalText1583 = "Ingen busser indenfor naeste time";
        }
      }
    }
    
    // Render stop 1583 name
    display.setFont(&FreeMonoBold12pt7b);
    display.getTextBounds(stopName1583.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2 - tbx;
    display.setCursor(x, y);
    display.print(stopName1583.c_str());
    
    // Render stop 1583 arrival information
    y += 30;  // Move down for arrival info
    display.setFont(&FreeMonoBold9pt7b);
    display.getTextBounds(arrivalText1583.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2 - tbx;
    display.setCursor(x, y);
    display.print(arrivalText1583.c_str());
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // BUS STOP 1550: Gammel Kongevej (H.C. Ørsteds Vej)  
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    y += 55;  // Move down with spacing between stops
    Serial.println("      • Processing bus stop 1550 data");
    
    // Extract stop name and arrival information
    String stopName1550 = "Stop 1550";  // Default fallback
    String arrivalText1550 = "No data available";  // Default fallback
    
    if (arrivals.containsKey("stop1550")) {
      JsonObject stop1550 = arrivals["stop1550"];
      if (stop1550.containsKey("name")) {
        stopName1550 = stop1550["name"].as<String>();
      }
      if (stop1550.containsKey("arrivals") && stop1550["arrivals"].is<JsonArray>()) {
        JsonArray arrivals1550 = stop1550["arrivals"];
        if (arrivals1550.size() > 0) {
          JsonObject firstArrival = arrivals1550[0];
          arrivalText1550 = firstArrival["formatted"].as<String>();
        } else {
          arrivalText1550 = "Ingen busser indenfor naeste time";
        }
      }
    }
    
    // Render stop 1550 name
    display.setFont(&FreeMonoBold12pt7b);
    display.getTextBounds(stopName1550.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2 - tbx;
    display.setCursor(x, y);
    display.print(stopName1550.c_str());
    
    // Render stop 1550 arrival information
    y += 30;  // Move down for arrival info
    display.setFont(&FreeMonoBold9pt7b);
    display.getTextBounds(arrivalText1550.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2 - tbx;
    display.setCursor(x, y);
    display.print(arrivalText1550.c_str());
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // CURRENT TIME DISPLAY
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    y += 55;  // Move down for current time section
    Serial.println("      • Rendering current time");
    
    // Format current time as "Current Time: HH:MM"
    String timeText = "Current Time: ";
    if (currentTime.hours >= 0 && currentTime.minutes >= 0) {
      if (currentTime.hours < 10) timeText += "0";
      timeText += String(currentTime.hours);
      timeText += ":";
      if (currentTime.minutes < 10) timeText += "0";
      timeText += String(currentTime.minutes);
    } else {
      timeText += "Unknown";
    }
    
    // Render current time
    display.setFont(&FreeMonoBold9pt7b);
    display.getTextBounds(timeText.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2 - tbx;
    display.setCursor(x, y);
    display.print(timeText.c_str());
    
  } while (display.nextPage());  // Complete the display update cycle
  
  Serial.println("   ✅ Display rendering complete - Bus arrival information updated");
}