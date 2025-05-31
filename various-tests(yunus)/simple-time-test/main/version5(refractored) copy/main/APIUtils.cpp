// File: APIUtils.cpp
// Purpose: Contains the logic for contacting the Rejseplanen API, processing bus
// arrival data, and sending it to the display module for rendering.

#include <ESP8266HTTPClient.h>  // HTTP client for ESP8266
#include <ESP8266WiFi.h>        // For WiFiClient and WiFi
#include <ArduinoJson.h>        // For JSON parsing
#include "WiFiUtils.h"          // For WiFi maintenance
#include "TimeUtils.h"          // For TimeData
#include "DisplayUtils.h"       // For display functions
#include "APIUtils.h"           // Declarations for this module

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

// Check bus arrivals from the API and prepare data for display
void checkBusArrivals(TimeData currentTime, GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>& display) {
    // Get current date and time for the API request
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        // Uncomment the following line to enable Serial debugging for time failure
        // Serial.println("Cannot fetch bus arrivals: no valid time");
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

    // Uncomment the following block to enable Serial debugging for API details
    /*
    Serial.println("--- API Debug Info ---");
    Serial.print("API URL: ");
    Serial.println(url);
    Serial.print("WiFi signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    */

    // Make the HTTP request to the API
    WiFiClient client;  // Create WiFiClient object
    HTTPClient http;
    http.begin(client, url);  // Updated API call
    int httpCode = http.GET();

    // Uncomment the following block to enable Serial debugging for HTTP response
    /*
    Serial.print("HTTP response code: ");
    Serial.println(httpCode);
    */

    if (httpCode == 200) {
        String payload = http.getString();
        // Uncomment the following block to enable Serial debugging for payload info
        /*
        Serial.print("Payload size (bytes): ");
        Serial.println(payload.length());
        Serial.print("Raw JSON snippet (first 100 chars): ");
        Serial.println(payload.substring(0, 100));
        */

        // Set memory for JSON parsing
        // Uses 4096 bytes, enough for most bus arrival data
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
            // Uncomment the following block to enable Serial debugging for JSON errors
            /*
            Serial.print("JSON parsing error: ");
            Serial.println(error.c_str());
            */
            http.end();
            // On error, keep the last display content
            return;
        }

        // Process the bus arrivals
        JsonArray arrivals = doc["Arrival"];
        // Uncomment the following block to enable Serial debugging for arrival count
        /*
        Serial.print("Number of arrivals processed: ");
        Serial.println(arrivals.size());
        */

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

        // Prepare arrival text for display
        String arrivalText1583, arrivalText1550;
        int diff1583 = nextTime1583 - currentMinutes;
        if (nextTime1583 < 1440 * 2 && diff1583 <= 60 && diff1583 >= 0) {
            if (diff1583 == 0) {
                arrivalText1583 = "Arriving now!";
            } else {
                arrivalText1583 = "+" + String(diff1583) + " min";
            }
            arrivalText1583 += " (" + nextArrival1583.substring(11, 16) + ")";
            arrivalText1583 += hasRealTime1583 ? " (RT)" : "";
        } else if (nextTime1583 >= 1440 * 2 || diff1583 > 60) {
            arrivalText1583 = "Ingen busser indenfor næste time";
        } else {
            arrivalText1583 = "Bus may have passed";
        }

        int diff1550 = nextTime1550 - currentMinutes;
        if (nextTime1550 < 1440 * 2 && diff1550 <= 60 && diff1550 >= 0) {
            if (diff1550 == 0) {
                arrivalText1550 = "Arriving now!";
            } else {
                arrivalText1550 = "+" + String(diff1550) + " min";
            }
            arrivalText1550 += " (" + nextArrival1550.substring(11, 16) + ")";
            arrivalText1550 += hasRealTime1550 ? " (RT)" : "";
        } else if (nextTime1550 >= 1440 * 2 || diff1550 > 60) {
            arrivalText1550 = "Ingen busser indenfor næste time";
        } else {
            arrivalText1550 = "Bus may have passed";
        }

        // Format current time for display
        char currentTimeDisplay[6];
        sprintf(currentTimeDisplay, "%02d:%02d", currentTime.hours, currentTime.minutes);

        // Send data to display
        drawBusStopDisplay(display, stopName1583, arrivalText1583, stopName1550, arrivalText1550, currentTimeDisplay);

        // Uncomment the following block to enable Serial debugging for bus arrivals
        /*
        Serial.println("────────── Next Bus Arrivals ──────────");
        Serial.print("Stop: ");
        Serial.println(stopName1583);
        Serial.println("  " + arrivalText1583);
        Serial.print("Stop: ");
        Serial.println(stopName1550);
        Serial.println("  " + arrivalText1550);
        Serial.println("───────────────────────────────────────");
        */
    } else {
        // Uncomment the following block to enable Serial debugging for HTTP errors
        /*
        Serial.print("HTTP error: ");
        Serial.println(httpCode);
        */
        // On error, keep the last display content
    }

    http.end();
}