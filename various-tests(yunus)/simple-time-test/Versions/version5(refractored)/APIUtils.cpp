// File: APIUtils.cpp
// Purpose: Contains the logic for contacting the Rejseplanen API, processing bus
// arrival data, and displaying it like a bus stop sign. Separates this from the main code.

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "WiFiUtils.h"  // Need WiFi maintenance
#include "TimeUtils.h"  // Need TimeData
#include "APIUtils.h"

// API details
const char* apiKey = "9b00b65e-e873-45af-8ff8-47366a137f53";
const char* stopIds = "1550|1583";

// Filter for JSON parsing to save memory
StaticJsonDocument<256> filter;

void setupAPIFilter() {
  // Set up the filter to grab only the bus data we need
  JsonObject arrivalFilter = filter["Arrival"].createNestedObject();
  arrivalFilter["stopExtId"] = true;  // Stop ID
  arrivalFilter["name"] = true;       // Line name
  arrivalFilter["time"] = true;       // Scheduled time
  arrivalFilter["date"] = true;       // Scheduled date
  arrivalFilter["rtTime"] = true;     // Real-time time
  arrivalFilter["rtDate"] = true;     // Real-time date
}

// Check bus arrivals from the API and show countdowns
void checkBusArrivals(TimeData currentTime) {
  // Get current date and time for the API request
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

  // Build the API URL to ask for bus data
  String url = "https://www.rejseplanen.dk/api/multiArrivalBoard?idList=" + String(stopIds) +
               "&date=" + currentDate + "&time=" + currentTimeStr + "&accessId=" + apiKey + "&format=json";

  // Debug: Show API and network details
  Serial.println("--- API Debug Info ---");
  Serial.print("API URL: ");
  Serial.println(url);
  Serial.print("WiFi signal strength (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  // Make the HTTP request to the API
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  Serial.print("HTTP response code: ");
  Serial.println(httpCode);

  if (httpCode == 200) {
    String payload = http.getString();
    Serial.print("Payload size (bytes): ");
    Serial.println(payload.length());
    Serial.print("Raw JSON snippet (first 100 chars): ");
    Serial.println(payload.substring(0, 100));

    // Set memory for JSON parsing
    // Uses 4096 bytes, enough for most bus arrival data from the API
    StaticJsonDocument<4096> doc;
    // OPTION 1: Use 2048 bytes instead
    // Why: Saves memory on devices with less space (e.g., Arduino Uno), but might fail if the API sends lots of data
    // How to test: Uncomment this, upload the code, and check Serial for "JSON parsing error"
    // StaticJsonDocument<2048> doc;
    // OPTION 2: Use 1024 bytes for very low memory
    // Why: Good for tiny devices, but very likely to fail if many bus arrivals are in the response
    // How to test: Uncomment this, watch Serial for errors, and increase size if needed
    // StaticJsonDocument<1024> doc;

    // Parse the JSON data from the API
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    if (error) {
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
      http.end();
      return;
    }

    // Process the bus arrivals
    JsonArray arrivals = doc["Arrival"];
    Serial.print("Number of arrivals processed: ");
    Serial.println(arrivals.size());

    // Set up variables for the next arrivals
    int nextTime1550 = 1440 * 2;  // Max minutes for two days
    String nextArrival1550;
    bool hasRealTime1550 = false;
    int nextTime1583 = 1440 * 2;
    String nextArrival1583;
    bool hasRealTime1583 = false;

    // Define stop names
    String stopName1550 = "Gammel Kongevej (H.C. Ørsteds Vej)";
    String stopName1583 = "Gammel Kongevej (Alhambravej)";

    for (JsonObject arrival : arrivals) {
      String stopExtId = arrival["stopExtId"].as<String>();
      String line = arrival["name"].as<String>();

      // Only look at "Bus 1A" arrivals
      if (line != "Bus 1A") continue;

      // Check if real-time data is available
      bool hasRealTime = arrival["rtTime"].is<String>() && arrival["rtDate"].is<String>();
      String arrivalDate = hasRealTime ? arrival["rtDate"].as<String>() : arrival["date"].as<String>();
      String arrivalTime = hasRealTime ? arrival["rtTime"].as<String>() : arrival["time"].as<String>();

      // Convert arrival time to minutes since midnight
      int arrivalHour = arrivalTime.substring(0, 2).toInt();
      int arrivalMinute = arrivalTime.substring(3, 5).toInt();
      int arrivalMinutes = arrivalHour * 60 + arrivalMinute;

      // Adjust for the next day if needed
      int arrivalTotalMinutes = arrivalMinutes;
      if (arrivalDate != String(currentDate)) {
        arrivalTotalMinutes += 1440;  // Add a day's worth of minutes
      }

      // Find the earliest arrival after current time
      if (arrivalTotalMinutes >= currentMinutes) {
        if (stopExtId == "1550" && arrivalTotalMinutes < nextTime1550) {
          nextTime1550 = arrivalTotalMinutes;
          nextArrival1550 = arrivalDate + " " + arrivalTime;
          hasRealTime1550 = hasRealTime;
        } else if (stopExtId == "1583" && arrivalTotalMinutes < nextTime1583) {
          nextTime1583 = arrivalTotalMinutes;
          nextArrival1583 = arrivalDate + " " + arrivalTime;
          hasRealTime1583 = hasRealTime;
        }
      }
    }

    // Show the results like a bus stop sign
    Serial.println("────────── Next Bus Arrivals ──────────");

    // Stop 1583
    Serial.print("Stop: ");
    Serial.println(stopName1583);
    int diff1583 = nextTime1583 - currentMinutes;
    if (nextTime1583 < 1440 * 2 && diff1583 <= 60 && diff1583 >= 0) {
      if (diff1583 == 0) {
        Serial.println("  Next bus (line 1A) is arriving now!");
      } else {
        Serial.print("  Next bus (line 1A) will be here in +");
        Serial.print(diff1583);
        Serial.println(" minutes");
      }
      Serial.print("    (Arrives ");
      Serial.print(nextArrival1583);
      Serial.print(hasRealTime1583 ? " - real-time estimate)" : " - scheduled estimate)");
      Serial.println();
    } else if (nextTime1583 >= 1440 * 2 || diff1583 > 60) {
      Serial.println("  Ingen busser ankommer indenfor den næste time.");
    } else {
      Serial.println("  Next bus (line 1A) may have just passed, checking next...");
    }

    // Stop 1550
    Serial.print("Stop: ");
    Serial.println(stopName1550);
    int diff1550 = nextTime1550 - currentMinutes;
    if (nextTime1550 < 1440 * 2 && diff1550 <= 60 && diff1550 >= 0) {
      if (diff1550 == 0) {
        Serial.println("  Next bus (line 1A) is arriving now!");
      } else {
        Serial.print("  Next bus (line 1A) will be here in +");
        Serial.print(diff1550);
        Serial.println(" minutes");
      }
      Serial.print("    (Arrives ");
      Serial.print(nextArrival1550);
      Serial.print(hasRealTime1550 ? " - real-time estimate)" : " - scheduled estimate)");
      Serial.println();
    } else if (nextTime1550 >= 1440 * 2 || diff1550 > 60) {
      Serial.println("  Ingen busser ankommer indenfor den næste time.");
    } else {
      Serial.println("  Next bus (line 1A) may have just passed, checking next...");
    }
    Serial.println("───────────────────────────────────────");
  } else {
    Serial.print("HTTP error: ");
    Serial.println(httpCode);
  }

  http.end();
}