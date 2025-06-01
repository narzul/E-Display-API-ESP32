#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "time.h"

// E-Paper display library
#include <GxEPD2_BW.h> // Main GxEPD2 library

// E-Paper Fonts - ensure these are available in your GxEPD2 library installation
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
// #include <Fonts/FreeMonoBold24pt7b.h> // Include if you need a larger font

// -------------------------------------------------------------------
// Configuration for ePaper Display
// -------------------------------------------------------------------
// Pin configuration for Arduino Nano ESP32 and 4.2" ePaper module GDEY042T81
// SPI pins (SCK, MOSI) are handled by the SPI library using default Nano ESP32 pins:
// SCK:  GPIO36 (Arduino pin SCK on Nano ESP32) - Connect to ePaper SCL
// MOSI: GPIO35 (Arduino pin MOSI on Nano ESP32) - Connect to ePaper SDA/DIN
// Make sure these are connected correctly.
// Control Pins for ePaper:
// These are Arduino D-pin numbers on the Nano ESP32.
// Ensure these pins are connected to the corresponding pins on your ePaper module.
const int EPD_CS_PIN   = D10; // Chip Select pin (GPIO21 on Nano ESP32)
const int EPD_DC_PIN   = D9;  // Data/Command pin (GPIO18 on Nano ESP32)
const int EPD_RST_PIN  = D8;  // Reset pin (GPIO17 on Nano ESP32)
const int EPD_BUSY_PIN = D7;  // Busy pin (GPIO10 on Nano ESP32)

// IMPORTANT: For SPI communication, you MUST also connect the following E-paper pins
// to the Nano ESP32's default SPI pins (which are not explicitly defined here but used by the library):
// E-paper SCL (SCK)   -> Arduino Nano ESP32 D12 (GPIO47)
// E-paper SDA (MOSI)  -> Arduino Nano ESP32 D11 (GPIO38)
// E-paper GND         -> Arduino Nano ESP32 GND
// E-paper VCC         -> Arduino Nano ESP32 3V3

// Initialize the display for the 4.2" ePaper module GDEY042T81 (400x300 pixels)
// The GxEPD2_420_GDEY042T81 class is specific to this model.
// GxEPD2_420_GDEY042T81::HEIGHT is 300.
// The display driver is GxEPD2_420_GDEY042T81.
// The display class is GxEPD2_BW (black and white).
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(
  EPD_CS_PIN, EPD_DC_PIN, EPD_RST_PIN, EPD_BUSY_PIN
));
// -------------------------------------------------------------------
// End of ePaper Display Configuration
// -------------------------------------------------------------------

// WiFi credentials
const char* ssid = "Yunes"; // Your WiFi SSID
const char* password = "123456789";     // Your WiFi Password

// API details
const char* apiKey = "9b00b65e-e873-45af-8ff8-47366a137f53"; // Your Rejseplanen API key
const char* stopIds = "1550|1583"; // Bus stop IDs

// NTP server settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;  // UTC+2 for CEST (Copenhagen time currently, no DST flag needed if NTP provides it)
const int   daylightOffset_sec = 0; // Set to 3600 if your GMT offset doesn't account for DST and you need manual offset

// Time structure
struct TimeData {
  int hours;
  int minutes;
  int seconds;
};

// Filter for JSON parsing to optimize memory usage
StaticJsonDocument<256> filter; // Filter definition for arrival data

// Function prototypes
TimeData getCurrentTimeFromNTP();
void maintainWiFi();
void printTime(TimeData time); // For Serial debugging
void checkBusArrivals(TimeData currentTime);
void updateEPaperDisplay(TimeData currentTime,
                         const String& stopName1583, int diff1583, const String& nextArrival1583, bool hasRealTime1583,
                         const String& stopName1550, int diff1550, const String& nextArrival1550, bool hasRealTime1550);

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for Serial to initialize

  Serial.println("Starting Bus Arrival Display (Single File Version)...");

  // Initialize the e-paper display
  // init(Serial debug bitrate, initial reset, busy timeout ms, partial update ram buffer size)
  display.init(115200, true, 50, false); 
  display.setRotation(1); // Set rotation to 1 (90 degrees counter-clockwise, portrait for 400x300 display -> 300x400)
  
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold12pt7b);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(10, 30);
    display.print("Initializing...");
    display.setCursor(10, 60);
    display.print("Connecting WiFi...");
  } while (display.nextPage());
  Serial.println("Display initialized.");

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  int wifi_retries = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_retries < 20) { // Retry for 10 seconds
    delay(500);
    Serial.print(".");
    wifi_retries++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    display.firstPage(); // Update display with WiFi status
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeMonoBold9pt7b); // Use a slightly smaller font for IP potentially
        display.setCursor(10,30);
        display.print("WiFi Connected!");
        display.setCursor(10,50);
        display.print("IP: ");
        display.print(WiFi.localIP());
    } while (display.nextPage());
    delay(2000); // Show message for a bit
  } else {
    Serial.println("WiFi connection FAILED");
     display.firstPage(); // Update display with WiFi status
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(10,30);
        display.print("WiFi Failed!");
    } while (display.nextPage());
    // Consider halting or periodic retries if WiFi is essential
  }


  // Configure NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Configuring time from NTP...");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10000)) { // Wait up to 10 seconds for first sync
    Serial.println("Initial NTP sync failed, will retry in loop...");
  } else {
    Serial.print("NTP sync successful. Current time: ");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }

  // Define the filter for JSON parsing (as in your original code)
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
  TimeData currentTime = getCurrentTimeFromNTP();

  // Process only if time is valid and the minute has changed
  if (currentTime.hours != -1 && currentTime.minutes != lastMinute) {
    lastMinute = currentTime.minutes;

    Serial.println("=== Time Info (Serial) ==="); // For debugging
    Serial.print("Current time: ");
    printTime(currentTime); // Serial print for debugging

    // Fetch and process bus arrivals, then update display
    checkBusArrivals(currentTime);

    Serial.println("==========================");
  }

  delay(1000); // Check every second (original was 100ms, 1s is fine for minute changes)
}

TimeData getCurrentTimeFromNTP() {
  TimeData timeData = {-1, -1, -1};
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time from NTP during loop check.");
    // Attempt to maintain WiFi and reconfigure time if needed, but don't block too long here
    maintainWiFi(); 
    // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // Could try re-config, but getLocalTime should keep trying
    return timeData; // Return invalid time if sync fails
  }

  timeData.hours = timeinfo.tm_hour;
  timeData.minutes = timeinfo.tm_min;
  timeData.seconds = timeinfo.tm_sec;

  return timeData;
}

void maintainWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    WiFi.disconnect(true); // Fully disconnect before trying again
    delay(100);
    WiFi.mode(WIFI_STA); // Ensure STA mode
    WiFi.begin(ssid, password);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 10) { // Try for 5 seconds
      delay(500);
      Serial.print(".");
      retries++;
    }
    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi reconnected.");
    } else {
        Serial.println("WiFi reconnection FAILED during maintainWiFi.");
    }
  }
}

// Print time in HH:MM:SS format to Serial (for debugging)
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

void checkBusArrivals(TimeData currentTime) {
  maintainWiFi(); // Ensure WiFi is active before API call
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No WiFi, cannot fetch bus data.");
    updateEPaperDisplay(currentTime, "WiFi Error", -1, "", false, "Check Connection", -1, "", false);
    return;
  }

  struct tm timeinfo;
  if (currentTime.hours == -1 || !getLocalTime(&timeinfo)) { // check if current time is valid
    Serial.println("Cannot fetch bus arrivals: no valid time from NTP.");
    updateEPaperDisplay(currentTime, "Time Error", -1, "", false, "NTP Sync Failed", -1, "", false);
    return;
  }
  char currentDate[11];
  strftime(currentDate, sizeof(currentDate), "%d.%m.%Y", &timeinfo); // Rejseplanen API date format DD.MM.YYYY
  char currentTimeStr[6];
  strftime(currentTimeStr, sizeof(currentTimeStr), "%H:%M", &timeinfo);
  int currentMinutes = currentTime.hours * 60 + currentTime.minutes;

  // Rejseplanen API expects date as DD.MM.YYYY and id (stop ID)
  // Using multiArrivalBoard, id is stopExtId for the specific platform/stop point
  // Example provided uses idList with pipe separated values.
  // The API documentation specifies `id` for `arrivalBoard` and `id` for `multiDepartureBoard`.
  // For `multiArrivalBoard`, it uses `id1`, `id2`, ... or `idList`.
  // Let's assume `idList` is correct as per original sketch.
  // Also, the date format for Rejseplanen API is typically DD.MM.YYYY
  // The original code used YYYY-MM-DD, which might be for a different API or an older version.
  // Rejseplanen official docs suggest DD.MM.YYYY for date parameter. I'll use that.
  // However, the user's original code had YYYY-MM-DD and it was "working" for serial print.
  // I will stick to YYYY-MM-DD format for `currentDate` as in the original working code for now to minimize changes.
  // If API fails with date, this should be changed to "%d.%m.%Y".
  strftime(currentDate, sizeof(currentDate), "%Y-%m-%d", &timeinfo); 

  String url = "https://xmlopen.rejseplanen.dk/bin/rest.exe/multiArrivalBoard?id=" + String(stopIds[0]) + 
               "&id=" + String(stopIds[1]) + // This is how multi ID often works if not idList
               // The user's original code used `idList=1550|1583`, which is likely correct for the specific endpoint
               // I'll revert to the user's idList format
               "&date=" + String(currentDate) + "&time=" + String(currentTimeStr) + 
               "&format=json";
               // The original URL was:
               // "https://www.rejseplanen.dk/api/multiArrivalBoard?idList=" + String(stopIds) +
               // "&date=" + currentDate + "&time=" + currentTimeStr + "&accessId=" + apiKey + "&format=json";
               // The `accessId` seems to be the API key.
               // The base URL might differ slightly. The original used `www.rejseplanen.dk/api/`
               // I will use the user's exact original URL structure.

  url = "https://www.rejseplanen.dk/api/multiArrivalBoard?idList=" + String(stopIds) +
        "&date=" + String(currentDate) + "&time=" + String(currentTimeStr) + 
        "&accessId=" + apiKey + "&format=json";


  Serial.println("--- Fetching Bus Data ---");
  Serial.print("API URL: "); Serial.println(url);

  HTTPClient http;
  http.setReuse(false); // Good practice
  http.begin(url); 
  int httpCode = http.GET();

  Serial.print("HTTP response code: "); Serial.println(httpCode);

  String nextArrival1550_str, nextArrival1583_str;
  int diff1550_val = -1, diff1583_val = -1; 
  bool hasRealTime1550_val = false, hasRealTime1583_val = false;
  String stopName1550 = "Gammel Kongevej (H.C. Ørsteds Vej)"; // Stop ID 1550
  String stopName1583 = "Gammel Kongevej (Alhambravej)";    // Stop ID 1583


  if (httpCode == HTTP_CODE_OK) { 
    String payload = http.getString();
    http.end(); // Close connection after getting payload

    Serial.print("Payload size (bytes): "); Serial.println(payload.length());
    if (payload.length() < 1000) { // Print small payloads for debugging
        // Serial.println("Raw JSON snippet: " + payload);
    } else {
        // Serial.println("Raw JSON snippet (first 500 chars): " + payload.substring(0,500));
    }


    // The payload from Rejseplanen multiArrivalBoard is wrapped in "MultiArrivalBoard": {"Arrival": [...]}
    // So the filter needs to be adjusted or the parsing needs to target the inner array.
    // Original filter: filter["Arrival"].createNestedObject(); -> This assumes "Arrival" is top-level array.
    // New filter strategy if payload is {"MultiArrivalBoard": {"Arrival": [...]}}
    // StaticJsonDocument<256> outer_filter;
    // outer_filter["MultiArrivalBoard"]["Arrival"].createNestedObject(); // this would work for filtering
    // For direct parsing, target doc["MultiArrivalBoard"]["Arrival"]
    
    // Sticking to original filter, assuming API structure is compatible or doc["Arrival"] path is valid from root.
    StaticJsonDocument<8192> doc; // Increased size for potentially larger Rejseplanen responses
    DeserializationError error = deserializeJson(doc, payload); // Try without filter first for debug
                                                                // DeserializationOption::Filter(filter)

    if (error) {
      Serial.print("JSON parsing error: "); Serial.println(error.c_str());
      updateEPaperDisplay(currentTime, "API Error", -1, "", false, "JSON Parse Fail", -1, "", false);
    } else {
      // Check if the structure is {"MultiArrivalBoard": {"Arrival": [...]}}
      JsonArray arrivals;
      if (doc.containsKey("MultiArrivalBoard") && doc["MultiArrivalBoard"].containsKey("Arrival")) {
        arrivals = doc["MultiArrivalBoard"]["Arrival"].as<JsonArray>();
      } else if (doc.containsKey("Arrival")) { // Or if "Arrival" is top-level as original code assumed
        arrivals = doc["Arrival"].as<JsonArray>();
      } else {
        Serial.println("JSON structure unexpected: 'Arrival' array not found at expected path.");
        updateEPaperDisplay(currentTime, "API Data Error", -1, "", false, "Bad JSON Struct", -1, "", false);
        return;
      }

      Serial.print("Number of arrivals processed: "); Serial.println(arrivals.size());

      int nextTime1550 = 1440 * 2 + currentMinutes +1; // Initialize to a value clearly "not found" and > current time
      int nextTime1583 = 1440 * 2 + currentMinutes +1;

      for (JsonObject arrival : arrivals) {
        String stopExtId = arrival["stopExtId"].as<String>(); // stopExtId is the platform ID
        String line = arrival["name"].as<String>(); // Bus line name, e.g., "Bus 1A"

        if (line != "Bus 1A") continue; 

        bool hasRealTime = arrival["rtTime"].is<String>() && !arrival["rtTime"].as<String>().isEmpty();
        String arrivalDateStr = hasRealTime ? arrival["rtDate"].as<String>() : arrival["date"].as<String>(); // DD.MM.YYYY
        String arrivalTimeStr = hasRealTime ? arrival["rtTime"].as<String>() : arrival["time"].as<String>(); // HH:MM

        // Convert arrival time (HH:MM) and date (DD.MM.YYYY) to minutes since midnight for comparison
        int arrivalHour = arrivalTimeStr.substring(0, 2).toInt();
        int arrivalMinute = arrivalTimeStr.substring(3, 5).toInt();
        int arrivalDay = arrivalDateStr.substring(0,2).toInt();
        int arrivalMonth = arrivalDateStr.substring(3,5).toInt();
        int arrivalYear = arrivalDateStr.substring(6,10).toInt();
        
        int arrivalMinutesToday = arrivalHour * 60 + arrivalMinute;
        
        int arrivalTotalMinutes = arrivalMinutesToday;

        // Compare dates properly for next-day arrivals
        // Current date from strftime("%Y-%m-%d", &timeinfo) is used for API call.
        // Arrival date from API is DD.MM.YYYY. Need to parse and compare.
        char currentApiDateFormatted[11]; // To match API's DD.MM.YYYY
        strftime(currentApiDateFormatted, sizeof(currentApiDateFormatted), "%d.%m.%Y", &timeinfo);

        if (arrivalDateStr != String(currentApiDateFormatted)) {
            // Simple check: if API date is greater than current formatted date string (lexicographically for DD.MM.YYYY)
            // This isn't perfectly robust for date comparison across month/year without proper date parsing.
            // A more robust way: convert both to epoch or compare year, then month, then day.
            // For simplicity, assuming if date string is different, it's the next day if time is "earlier".
            // If arrivalHour is e.g. 01:00 and current time is 23:00, it's likely next day.
            if (arrivalHour < currentTime.hours && (currentTime.hours - arrivalHour > 12)) { // Heuristic
                 arrivalTotalMinutes += 1440; 
            } else {
                // Basic check: If the arrival year/month/day is later than current
                int currentDay = timeinfo.tm_mday;
                int currentMonth = timeinfo.tm_mon + 1; // tm_mon is 0-11
                int currentYear = timeinfo.tm_year + 1900; // tm_year is years since 1900

                if (arrivalYear > currentYear ||
                    (arrivalYear == currentYear && arrivalMonth > currentMonth) ||
                    (arrivalYear == currentYear && arrivalMonth == currentMonth && arrivalDay > currentDay)) {
                    arrivalTotalMinutes += 1440; // Add a day's worth of minutes
                }
            }
        }


        if (arrivalTotalMinutes >= currentMinutes) { 
          if (stopExtId == "000011550" && arrivalTotalMinutes < nextTime1550) { // API often uses leading zeros for stopExtId
            nextTime1550 = arrivalTotalMinutes;
            nextArrival1550_str = arrivalDateStr + " " + arrivalTimeStr;
            hasRealTime1550_val = hasRealTime;
          } else if (stopExtId == "000011583" && arrivalTotalMinutes < nextTime1583) {
            nextTime1583 = arrivalTotalMinutes;
            nextArrival1583_str = arrivalDateStr + " " + arrivalTimeStr;
            hasRealTime1583_val = hasRealTime;
          }
          // Fallback if stop IDs don't have leading zeros in API response but are "1550"
          else if (stopExtId == "1550" && arrivalTotalMinutes < nextTime1550) {
            nextTime1550 = arrivalTotalMinutes;
            nextArrival1550_str = arrivalDateStr + " " + arrivalTimeStr;
            hasRealTime1550_val = hasRealTime;
          } else if (stopExtId == "1583" && arrivalTotalMinutes < nextTime1583) {
            nextTime1583 = arrivalTotalMinutes;
            nextArrival1583_str = arrivalDateStr + " " + arrivalTimeStr;
            hasRealTime1583_val = hasRealTime;
          }
        }
      }

      if (!nextArrival1550_str.isEmpty()) {
        diff1550_val = nextTime1550 - currentMinutes;
      }
      if (!nextArrival1583_str.isEmpty()) {
        diff1583_val = nextTime1583 - currentMinutes;
      }
      
      Serial.println("--- Processed Bus Info ---");
      Serial.printf("Stop 1583 (%s): Diff: %d min, Arrival: %s, RT: %s\n", stopName1583.c_str(), diff1583_val, nextArrival1583_str.c_str(), hasRealTime1583_val ? "Yes" : "No");
      Serial.printf("Stop 1550 (%s): Diff: %d min, Arrival: %s, RT: %s\n", stopName1550.c_str(), diff1550_val, nextArrival1550_str.c_str(), hasRealTime1550_val ? "Yes" : "No");

      updateEPaperDisplay(currentTime, stopName1583, diff1583_val, nextArrival1583_str, hasRealTime1583_val,
                                     stopName1550, diff1550_val, nextArrival1550_str, hasRealTime1550_val);
    }
  } else {
    Serial.print("HTTP error: "); Serial.println(httpCode);
    http.end(); // Ensure connection is closed on error too
    String httpErrorMsg = "HTTP Error: " + String(httpCode);
    updateEPaperDisplay(currentTime, httpErrorMsg, -1, "", false, "", -1, "", false);
  }
  // http.end(); // Already called in HTTP_CODE_OK and error branches if http object is stack allocated per call
}


void updateEPaperDisplay(TimeData currentTime,
                         const String& stopName1583, int diff1583, const String& nextArrival1583, bool hasRealTime1583,
                         const String& stopName1550, int diff1550, const String& nextArrival1550, bool hasRealTime1550) {
  Serial.println("Updating e-paper display (simplified)...");
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    int16_t yPos = 20; // Initial Y position

    // Display Current Time
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(10, yPos);
    char timeStr[10];
    if (currentTime.hours != -1) {
        sprintf(timeStr, "%02d:%02d:%02d", currentTime.hours, currentTime.minutes, currentTime.seconds);
    } else {
        sprintf(timeStr, "--:--:--");
    }
    display.print("Time: ");
    display.println(timeStr);
    yPos += 25; // Move Y down for next line (approx height for 12pt + spacing)

    // --- Display Bus Stop 1583 (Alhambravej) ---
    display.setFont(&FreeMonoBold9pt7b); // Switch to 9pt font
    display.setCursor(5, yPos);
    // Keep stop names short for this simple version, or print only part of it
    if (stopName1583.startsWith("WiFi Error") || stopName1583.startsWith("HTTP Error") || stopName1583.startsWith("API Error") || stopName1583.startsWith("Time Error")) {
        display.println(stopName1583.substring(0,35)); // Print error message
    } else {
        display.println("Stop Alhambravej (1583)"); // Simplified name
    }
    yPos += 18; // Move Y down (approx height for 9pt + spacing)

    display.setCursor(15, yPos); // Indent details
    if (stopName1583.startsWith("WiFi Error") || stopName1583.startsWith("HTTP Error") || stopName1583.startsWith("API Error") || stopName1583.startsWith("Time Error")){
        // Error already printed
    } else if (nextArrival1583.isEmpty()) {
      display.println(" 1A: No upcoming bus.");
    } else if (diff1583 > 60) {
      display.println(" 1A: Not within hour.");
    } else if (diff1583 < 0) {
      display.println(" 1A: Passed/check data.");
    } else if (diff1583 == 0) {
      display.println(" 1A: Arriving NOW!");
    } else {
      display.print(" 1A: in ");
      display.print(diff1583);
      display.println(" min");
    }
    yPos += 18; // Move Y down

    if (!nextArrival1583.isEmpty() && diff1583 >= 0 && diff1583 <= 60 &&
        !(stopName1583.startsWith("WiFi Error") || stopName1583.startsWith("HTTP Error") || stopName1583.startsWith("API Error") || stopName1583.startsWith("Time Error"))) {
      display.setCursor(15, yPos);
      String arrivalTimeOnly = nextArrival1583.substring(nextArrival1583.indexOf(" ") + 1);
      display.print("   ("); // Extra indent for arrival time
      display.print(arrivalTimeOnly);
      display.print(hasRealTime1583 ? " RT" : " SCH");
      display.println(")");
      yPos += 18; // Move Y down
    }
    yPos += 10; // Extra space before next stop

    // --- Display Bus Stop 1550 (H.C. Ørsteds Vej) ---
    display.setCursor(5, yPos);
     if (stopName1550.startsWith("Check Connection") || stopName1550.startsWith("JSON Parse Fail") || stopName1550.startsWith("NTP Sync Failed") || stopName1550.startsWith("Bad JSON Struct")) {
        display.println(stopName1550.substring(0,35)); // Print error message
    } else {
        display.println("Stop HC Orsteds Vej (1550)"); // Simplified name
    }
    yPos += 18; // Move Y down

    display.setCursor(15, yPos); // Indent details
    if (stopName1550.startsWith("Check Connection") || stopName1550.startsWith("JSON Parse Fail") || stopName1550.startsWith("NTP Sync Failed") || stopName1550.startsWith("Bad JSON Struct")) {
        // Error already printed
    } else if (nextArrival1550.isEmpty()) {
      display.println(" 1A: No upcoming bus.");
    } else if (diff1550 > 60) {
      display.println(" 1A: Not within hour.");
    } else if (diff1550 < 0) {
      display.println(" 1A: Passed/check data.");
    } else if (diff1550 == 0) {
      display.println(" 1A: Arriving NOW!");
    } else {
      display.print(" 1A: in ");
      display.print(diff1550);
      display.println(" min");
    }
    yPos += 18; // Move Y down

    if (!nextArrival1550.isEmpty() && diff1550 >= 0 && diff1550 <= 60 &&
        !(stopName1550.startsWith("Check Connection") || stopName1550.startsWith("JSON Parse Fail") || stopName1550.startsWith("NTP Sync Failed") || stopName1550.startsWith("Bad JSON Struct"))) {
      display.setCursor(15, yPos);
      String arrivalTimeOnly = nextArrival1550.substring(nextArrival1550.indexOf(" ") + 1);
      display.print("   ("); // Extra indent
      display.print(arrivalTimeOnly);
      display.print(hasRealTime1550 ? " RT" : " SCH");
      display.println(")");
      yPos += 18; // Move Y down
    }
    
    // Last update timestamp at the bottom (simplified position)
    display.setFont(&FreeMonoBold9pt7b);
    // display.height() should be 400 if rotation is 1 (portrait)
    // Place it near bottom, e.g. at y = 360
    if (display.height() > 50) { // Basic check if height is sensible
        display.setCursor(10, display.height() - 30);
        display.print("Update: ");
        display.print(timeStr);
    }


  } while (display.nextPage());
  Serial.println("E-Paper display update complete (simplified).");
}