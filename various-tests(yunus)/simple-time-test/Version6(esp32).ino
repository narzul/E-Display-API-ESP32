#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include "time.h"

// WiFi credentials
const char* ssid = "Fibernet-60713347";
const char* password = "f5567r6k";

// API details
const char* apiKey = "9b00b65e-e873-45af-8ff8-47366a137f53";
const char* stopIds = "1550|1583";

// NTP server settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;  // UTC+2 for CEST
const int   daylightOffset_sec = 0;

// E-paper display configuration for Arduino Nano ESP32
// Adjusted pin assignments for Nano ESP32 pinout
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(
  /*CS=*/   D10,   // Chip Select pin (GPIO 10)
  /*DC=*/   D9,    // Data/Command pin (GPIO 9)
  /*RST=*/  D8,    // Reset pin (GPIO 8)
  /*BUSY=*/ D7     // Busy pin (GPIO 7)
  // SCK (Serial Clock) is connected to D13 (GPIO 13)
  // MOSI (Serial Data) is connected to D11 (GPIO 11)
));

// Time structure
struct TimeData {
  int hours;
  int minutes;
  int seconds;
};

// Bus stop information structure
struct BusStopInfo {
  String stopId;
  String stopName;
  int nextArrivalMinutes;
  String nextArrivalTime;
  bool hasRealTime;
  bool hasValidArrival;
};

// Filter for JSON parsing to optimize memory usage
StaticJsonDocument<256> filter;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize the e-paper display
  display.init(115200, true, 50, false);
  display.setRotation(0); // Portrait orientation
  
  // Show startup message
  showStartupMessage();

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Configure NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(1000);  // Give time to sync
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Initial NTP sync failed, retrying in loop...");
  }

  // Define the filter for JSON parsing
  JsonObject arrivalFilter = filter["Arrival"].createNestedObject();
  arrivalFilter["stopExtId"] = true;  // Stop ID
  arrivalFilter["name"] = true;       // Line name
  arrivalFilter["time"] = true;       // Scheduled time
  arrivalFilter["date"] = true;       // Scheduled date
  arrivalFilter["rtTime"] = true;     // Real-time time
  arrivalFilter["rtDate"] = true;     // Real-time date
}

void loop() {
  static int lastMinute = -1;
  static unsigned long lastDisplayUpdate = 0;
  const unsigned long DISPLAY_UPDATE_INTERVAL = 60000; // Update display every minute
  
  TimeData currentTime = getCurrentTimeFromNTP();

  // Process only if time is valid and the minute has changed
  if (currentTime.hours != -1 && currentTime.minutes != lastMinute) {
    lastMinute = currentTime.minutes;

    // Display time information
    Serial.println("=== Time Info ===");
    Serial.print("Current time: ");
    printTime(currentTime);

    // Fetch and process bus arrivals
    BusStopInfo stops[2];
    checkBusArrivals(currentTime, stops);

    // Update display
    updateDisplay(currentTime, stops);
    lastDisplayUpdate = millis();

    Serial.println("=================");
  }

  delay(100);  // Check 10 times per second to catch minute changes
}

void showStartupMessage() {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMonoBold18pt7b);
    display.setTextColor(GxEPD_BLACK);
    
    int16_t tbx, tby; 
    uint16_t tbw, tbh;
    String msg = "Bus Tracker";
    display.getTextBounds(msg.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = (display.height() / 2) - tby;
    
    display.setCursor(x, y);
    display.print(msg);
    
    display.setFont(&FreeMonoBold12pt7b);
    String msg2 = "Starting...";
    display.getTextBounds(msg2.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = ((display.width() - tbw) / 2) - tbx;
    y += 50;
    display.setCursor(x, y);
    display.print(msg2);
    
  } while (display.nextPage());
}

void updateDisplay(TimeData currentTime, BusStopInfo stops[2]) {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    // Display title and current time
    display.setFont(&FreeMonoBold18pt7b);
    int16_t tbx, tby; 
    uint16_t tbw, tbh;
    String title = "Bus 1A Arrivals";
    display.getTextBounds(title.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = 40;
    display.setCursor(x, y);
    display.print(title);
    
    // Display current time
    display.setFont(&FreeMonoBold12pt7b);
    String timeStr = String(currentTime.hours < 10 ? "0" : "") + String(currentTime.hours) + ":" +
                     String(currentTime.minutes < 10 ? "0" : "") + String(currentTime.minutes);
    display.getTextBounds(timeStr.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = ((display.width() - tbw) / 2) - tbx;
    y += 40;
    display.setCursor(x, y);
    display.print(timeStr);
    
    // Draw separator line
    y += 20;
    display.drawLine(20, y, display.width() - 20, y, GxEPD_BLACK);
    y += 30;
    
    // Display bus stop information
    for (int i = 0; i < 2; i++) {
      displayBusStopInfo(stops[i], y);
      y += 80; // Space between stops
    }
    
  } while (display.nextPage());
}

void displayBusStopInfo(BusStopInfo stop, uint16_t startY) {
  display.setFont(&FreeMonoBold12pt7b);
  
  // Stop name (truncated if too long)
  String displayName = stop.stopName;
  if (displayName.length() > 25) {
    displayName = displayName.substring(0, 22) + "...";
  }
  
  display.setCursor(10, startY);
  display.print(displayName);
  
  // Next arrival info
  display.setFont(&FreeMonoBold9pt7b);
  uint16_t y = startY + 25;
  
  if (stop.hasValidArrival && stop.nextArrivalMinutes <= 60 && stop.nextArrivalMinutes >= 0) {
    if (stop.nextArrivalMinutes == 0) {
      display.setCursor(10, y);
      display.print("Arriving NOW!");
    } else {
      display.setCursor(10, y);
      display.print("In " + String(stop.nextArrivalMinutes) + " minutes");
    }
    
    y += 20;
    display.setCursor(10, y);
    display.print("At " + stop.nextArrivalTime);
    display.print(stop.hasRealTime ? " (RT)" : " (SCH)");
  } else {
    display.setCursor(10, y);
    display.print("No arrivals in next hour");
  }
}

// Get current time from NTP with error handling
TimeData getCurrentTimeFromNTP() {
  TimeData timeData = {-1, -1, -1};
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time, attempting to resync...");
    maintainWiFi();  // Ensure WiFi is connected
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(1000);  // Wait a second for sync
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Still failed to obtain time.");
      return timeData;  // Return invalid time if sync fails
    }
  }

  timeData.hours = timeinfo.tm_hour;
  timeData.minutes = timeinfo.tm_min;
  timeData.seconds = timeinfo.tm_sec;

  return timeData;
}

// Maintain WiFi connection
void maintainWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("WiFi reconnected.");
  }
}

// Print time in HH:MM:SS format
void printTime(TimeData time) {
  if (time.hours < 10) Serial.print("0");
  Serial.print(time.hours);
  Serial.print(":");
  if (time.minutes < 10) Serial.print("0");
  Serial.print(time.minutes);
  Serial.print(":");
  if (time.seconds < 10) Serial.print("0");
  Serial.println(time.seconds);
}

// Check bus arrivals from Rejseplanen API and calculate countdown
void checkBusArrivals(TimeData currentTime, BusStopInfo stops[2]) {
  // Initialize stop information
  stops[0] = {"1550", "Gammel Kongevej (H.C. Orsteds Vej)", 1440*2, "", false, false};
  stops[1] = {"1583", "Gammel Kongevej (Alhambravej)", 1440*2, "", false, false};
  
  // Extract current date and time for API
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Cannot fetch bus arrivals: no valid time");
    return;
  }
  char currentDate[11];
  strftime(currentDate, sizeof(currentDate), "%Y-%m-%d", &timeinfo);
  char currentTimeStr[6];
  strftime(currentTimeStr, sizeof(currentTimeStr), "%H:%M", &timeinfo);
  int currentMinutes = currentTime.hours * 60 + currentTime.minutes;

  // Construct API URL
  String url = "https://www.rejseplanen.dk/api/multiArrivalBoard?idList=" + String(stopIds) +
               "&date=" + currentDate + "&time=" + currentTimeStr + "&accessId=" + apiKey + "&format=json";

  // Debug: Log API interaction and network details
  Serial.println("--- API Debug Info ---");
  Serial.print("API URL: ");
  Serial.println(url);
  Serial.print("WiFi signal strength (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  // Make HTTP request
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  Serial.print("HTTP response code: ");
  Serial.println(httpCode);

  if (httpCode == 200) {
    String payload = http.getString();
    Serial.print("Payload size (bytes): ");
    Serial.println(payload.length());

    // Parse JSON
    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    if (error) {
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
      http.end();
      return;
    }

    // Process arrivals
    JsonArray arrivals = doc["Arrival"];
    Serial.print("Number of arrivals processed: ");
    Serial.println(arrivals.size());

    for (JsonObject arrival : arrivals) {
      String stopExtId = arrival["stopExtId"].as<String>();
      String line = arrival["name"].as<String>();

      // Only process arrivals for "Bus 1A"
      if (line != "Bus 1A") continue;

      // Check for real-time data availability
      bool hasRealTime = arrival["rtTime"].is<String>() && arrival["rtDate"].is<String>();
      String arrivalDate = hasRealTime ? arrival["rtDate"].as<String>() : arrival["date"].as<String>();
      String arrivalTime = hasRealTime ? arrival["rtTime"].as<String>() : arrival["time"].as<String>();

      // Convert arrival time to minutes since midnight
      int arrivalHour = arrivalTime.substring(0, 2).toInt();
      int arrivalMinute = arrivalTime.substring(3, 5).toInt();
      int arrivalMinutes = arrivalHour * 60 + arrivalMinute;

      // Adjust for next day if necessary
      int arrivalTotalMinutes = arrivalMinutes;
      if (arrivalDate != String(currentDate)) {
        arrivalTotalMinutes += 1440;  // Add a day's worth of minutes
      }

      // Find the earliest arrival after current time for each stop
      if (arrivalTotalMinutes >= currentMinutes) {
        int stopIndex = (stopExtId == "1550") ? 0 : 1;
        if (arrivalTotalMinutes < stops[stopIndex].nextArrivalMinutes) {
          stops[stopIndex].nextArrivalMinutes = arrivalTotalMinutes - currentMinutes;
          stops[stopIndex].nextArrivalTime = arrivalTime;
          stops[stopIndex].hasRealTime = hasRealTime;
          stops[stopIndex].hasValidArrival = true;
        }
      }
    }

    // Display formatted results in Serial
    Serial.println("────────── Next Bus Arrivals ──────────");
    for (int i = 0; i < 2; i++) {
      Serial.print("Bus stoppested: ");
      Serial.println(stops[i].stopName);
      
      if (stops[i].hasValidArrival && stops[i].nextArrivalMinutes <= 60 && stops[i].nextArrivalMinutes >= 0) {
        if (stops[i].nextArrivalMinutes == 0) {
          Serial.println("  Next bus (line 1A) is arriving now!");
        } else {
          Serial.print("  Next bus (line 1A) will be here in +");
          Serial.print(stops[i].nextArrivalMinutes);
          Serial.println(" minutes");
        }
        Serial.print("    (Arrives ");
        Serial.print(stops[i].nextArrivalTime);
        Serial.print(stops[i].hasRealTime ? " - real-time estimate)" : " - scheduled estimate)");
        Serial.println();
      } else {
        Serial.println("  Ingen busser ankommer indenfor den næste time.");
      }
    }
    Serial.println("───────────────────────────────────────");
  } else {
    Serial.print("HTTP error: ");
    Serial.println(httpCode);
  }

  http.end();
}