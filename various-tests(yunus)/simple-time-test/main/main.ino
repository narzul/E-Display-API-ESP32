#include <WiFi.h>
#include "time.h"

// WiFi credentials
const char* ssid = "Labitat (free)";
const char* password = "labitatisawesome";

// NTP server settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

// Time structure
struct TimeData {
  int hours;
  int minutes;
  int seconds;
};

// === CHANGE ME ===
// Target time to compare against (in future this can be dynamically set)
TimeData targetTime = {11, 11, 24};

// Internal tracking
int lastProcessedMinute = -1;
unsigned long lastSyncTime = 0;
const unsigned long syncInterval = 3600000; // 1 hour in milliseconds

void setup() {
  Serial.begin(115200);
  delay(1000);

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
  lastSyncTime = millis();
}

void loop() {
  // Periodic NTP re-sync (every hour)
  if (millis() - lastSyncTime >= syncInterval) {
    Serial.println("Resyncing time with NTP...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    lastSyncTime = millis();
  }

  // Get current time
  TimeData currentTime = getCurrentTimeFromNTP();

  // Only process if a new minute has started and we’re within the first second
  if (currentTime.minutes != lastProcessedMinute && currentTime.seconds <= 1) {
    lastProcessedMinute = currentTime.minutes;

    Serial.print("Current time: ");
    printTime(currentTime);

    Serial.print("Target time:  ");
    printTime(targetTime);

    int minuteDifference = calculateMinuteDifference(currentTime, targetTime);

    Serial.print("Time difference: ");
    if (minuteDifference >= 0) Serial.print("+");
    Serial.print(minuteDifference);
    Serial.println(" minutes");
    Serial.println("------------------------");
  }

  delay(100); // Check 10 times per second
}

TimeData getCurrentTimeFromNTP() {
  TimeData timeData = {-1, -1, -1};
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    return timeData;
  }

  timeData.hours = timeinfo.tm_hour;
  timeData.minutes = timeinfo.tm_min;
  timeData.seconds = timeinfo.tm_sec;

  return timeData;
}

int calculateMinuteDifference(TimeData time1, TimeData time2) {
  int totalMinutes1 = time1.hours * 60 + time1.minutes;
  int totalMinutes2 = time2.hours * 60 + time2.minutes;
  return totalMinutes2 - totalMinutes1;
}

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
