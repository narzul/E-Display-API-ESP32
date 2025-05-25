#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Time API endpoint (WorldTimeAPI - free and reliable)
const char* timeAPI = "http://worldtimeapi.org/api/timezone/UTC";

// Structure to hold time data
struct TimeData {
  int hours;
  int minutes;
  int seconds;
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Fetch current time from API
  TimeData currentTime = getCurrentTime();
  
  if (currentTime.hours != -1) { // Check if time fetch was successful
    Serial.print("Current time: ");
    printTime(currentTime);
    
    // Example: Compare with another time (11:11:24)
    TimeData otherTime = {11, 11, 24};
    Serial.print("Other time: ");
    printTime(otherTime);
    
    // Calculate difference in minutes
    int minuteDifference = calculateMinuteDifference(currentTime, otherTime);
    
    if (minuteDifference >= 0) {
      Serial.print("Time difference: +");
      Serial.print(minuteDifference);
      Serial.println(" minutes");
    } else {
      Serial.print("Time difference: ");
      Serial.print(minuteDifference);
      Serial.println(" minutes");
    }
    
    Serial.println("------------------------");
  } else {
    Serial.println("Failed to fetch time from API");
  }
  
  delay(10000); // Wait 10 seconds before next fetch
}

TimeData getCurrentTime() {
  TimeData timeData = {-1, -1, -1}; // Initialize with error values
  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(timeAPI);
    
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("API Response received");
      
      // Parse JSON response
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      
      // Extract datetime string (format: 2024-01-15T10:56:23.123456+00:00)
      String datetime = doc["datetime"];
      
      // Extract time part (after 'T' and before '.')
      int tIndex = datetime.indexOf('T');
      int dotIndex = datetime.indexOf('.');
      
      if (tIndex != -1 && dotIndex != -1) {
        String timeString = datetime.substring(tIndex + 1, dotIndex);
        
        // Parse hours, minutes, seconds
        timeData.hours = timeString.substring(0, 2).toInt();
        timeData.minutes = timeString.substring(3, 5).toInt();
        timeData.seconds = timeString.substring(6, 8).toInt();
      }
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
  
  return timeData;
}

int calculateMinuteDifference(TimeData time1, TimeData time2) {
  // Convert both times to total minutes since midnight
  int totalMinutes1 = time1.hours * 60 + time1.minutes;
  int totalMinutes2 = time2.hours * 60 + time2.minutes;
  
  // Add seconds contribution (rounded down to nearest minute)
  // We don't add anything for seconds since we're rounding down
  
  // Calculate difference
  int difference = totalMinutes2 - totalMinutes1;
  
  return difference;
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

// Helper function to create TimeData from individual values
TimeData createTime(int hours, int minutes, int seconds) {
  TimeData time;
  time.hours = hours;
  time.minutes = minutes;
  time.seconds = seconds;
  return time;
}

// Helper function to get time difference in a more readable format
void printTimeDifference(TimeData time1, TimeData time2) {
  int minuteDiff = calculateMinuteDifference(time1, time2);
  
  Serial.print("Time difference between ");
  printTime(time1);
  Serial.print(" and ");
  printTime(time2);
  Serial.print(" is: ");
  
  if (minuteDiff >= 0) {
    Serial.print("+");
  }
  Serial.print(minuteDiff);
  Serial.println(" minutes");
}