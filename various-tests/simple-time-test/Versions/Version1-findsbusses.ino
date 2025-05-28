// Define a higher nesting limit to handle deep JSON structures
#define ARDUINOJSON_DEFAULT_NESTING_LIMIT 16

// Include necessary libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// WiFi credentials (replace with your own)
const char* ssid = "Labitat (free)";
const char* password = "labitatisawesome";

// API details
const char* apiKey = "9b00b65e-e873-45af-8ff8-47366a137f53";
const char* stopIds = "1550|1583";

// Filter for JSON parsing to optimize memory usage
StaticJsonDocument<256> filter;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set time via NTP (UTC+2 for CEST)
  configTime(7200, 0, "pool.ntp.org");

  // Wait for time to synchronize
  while (time(nullptr) < 100000) {
    delay(1000);
    Serial.println("Waiting for time sync...");
  }

  // Define the filter for JSON parsing
  JsonObject arrivalFilter = filter["Arrival"].createNestedObject();
  arrivalFilter["stopExtId"] = true;  // Stop ID
  arrivalFilter["name"] = true;       // Line name
  arrivalFilter["time"] = true;       // Scheduled time
  arrivalFilter["date"] = true;       // Scheduled date
  arrivalFilter["rtTime"] = true;     // Real-time time (if available)
  arrivalFilter["rtDate"] = true;     // Real-time date (if available)
}

void loop() {
  // Get current time
  time_t now = time(nullptr);
  struct tm *localTime = localtime(&now);

  // Extract current date and time
  char currentDate[11];
  strftime(currentDate, sizeof(currentDate), "%Y-%m-%d", localTime);
  char currentTime[6];
  strftime(currentTime, sizeof(currentTime), "%H:%M", localTime);
  int currentHour = localTime->tm_hour;
  int currentMinute = localTime->tm_min;
  int currentMinutes = currentHour * 60 + currentMinute;

  // Construct API URL
  String url = "https://www.rejseplanen.dk/api/multiArrivalBoard?idList=" + String(stopIds) +
               "&date=" + currentDate + "&time=" + currentTime + "&accessId=" + apiKey + "&format=json";

  // Make HTTP request
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();

    // Parse JSON with filter and increased document size
    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    if (error) {
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
      return;
    }

    // Process arrivals
    JsonArray arrivals = doc["Arrival"];
    int nextTime1550 = 1440; // Max minutes in a day (initially set to a large value)
    int nextTime1583 = 1440;
    String nextLine1550, nextLine1583;

    for (JsonObject arrival : arrivals) {
      String stopExtId = arrival["stopExtId"].as<String>();
      String line = arrival["name"].as<String>();
      String timeStr = arrival["rtTime"] | arrival["time"]; // Use real-time if available, else scheduled

      // Convert arrival time to minutes since midnight
      int arrivalHour = timeStr.substring(0, 2).toInt();
      int arrivalMinute = timeStr.substring(3, 5).toInt();
      int arrivalMinutes = arrivalHour * 60 + arrivalMinute;

 - 1;

      // Update next arrival time for stop 1550
      if (stopExtId == "1550" && arrivalMinutes >= currentMinutes && arrivalMinutes < nextTime1550) {
        nextTime1550 = arrivalMinutes;
        nextLine1550 = line;
      }
      // Update next arrival time for stop 1583
      else if (stopExtId == "1583" && arrivalMinutes >= currentMinutes && arrivalMinutes < nextTime1583) {
        nextTime1583 = arrivalMinutes;
        nextLine1583 = line;
      }
    }

    // Display results for stop 1550
    if (nextTime1550 < 1440) {
      int diff = nextTime1550 - currentMinutes;
      Serial.print("Stop 1550: Next bus (");
      Serial.print(nextLine1550);
      Serial.print(") in ");
      Serial.print(diff);
      Serial.println(" minutes");
    } else {
      Serial.println("Stop 1550: No more buses today");
    }

    // Display results for stop 1583
    if (nextTime1583 < 1440) {
      int diff = nextTime1583 - currentMinutes;
      Serial.print("Stop 1583: Next bus (");
      Serial.print(nextLine1583);
      Serial.print(") in ");
      Serial.print(diff);
      Serial.println(" minutes");
    } else {
      Serial.println("Stop 1583: No more buses today");
    }
  } else {
    Serial.print("HTTP error: ");
    Serial.println(httpCode);
  }

  http.end();
  delay(60000); // Update every minute
}