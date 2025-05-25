/*
*
* Made by Jonas Kjeldmnd Jensen (Yunus), September 2024
* Extended with time management functionality
*
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

// Load in other header files from the project folder
#include "DisplayConfig.h"
#include "Credentials.h"
#include "TimeManager.h"

// Replace with the city you want to get the weather for
const char* city = "Copenhagen";

void setup()
{
  Serial.begin(115200);
  delay(100);

  // Initialize the display
  display.init(115200, true, 50, false);
  display.setRotation(4); // Set the rotation to 90 degrees counter-clockwise

  // Connect to WiFi
  connectToWifi();

  // Initialize time from NTP server
  if (initializeTime()) {
    // Store the initial time
    storeCurrentTime();
    
    // Show current time for demonstration
    TimeInfo currentTime = getCurrentTime();
    Serial.println("Setup completed at: " + formatTime(currentTime));
  }

  // Fetch and display the weather data
  fetchAndDisplayWeatherData();
}

void connectToWifi()
{
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void fetchAndDisplayWeatherData()
{
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String apiUrl = "https://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&appid=" + String(apiKey) + "&units=metric";
    http.begin(apiUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      String payload = http.getString();
      Serial.println("Received payload: " + payload);

      // Parse the JSON data
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      // Extract the relevant data
      String cityName = doc["name"].as<String>();
      float temperature = doc["main"]["temp"].as<float>();
      String description = doc["weather"][0]["description"].as<String>();

      // Get current time information
      TimeInfo currentTime = getCurrentTime();
      TimeInfo storedTime = getStoredTime();
      int timeDifference = getTimeDifferenceMinutes(storedTime, currentTime);

      // Display the data on the e-paper display including time info
      displayWeatherAndTime(cityName, temperature, description, currentTime, timeDifference);
      
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void displayWeatherAndTime(String cityName, float temperature, String description, TimeInfo currentTime, int timeDiff) {
  display.setFont(&FreeMonoBold24pt7b);
  display.setTextColor(GxEPD_BLACK);

  int16_t tbx, tby; 
  uint16_t tbw, tbh;
  display.getTextBounds(cityName.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
  
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = 50;  // Start from top

  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    
    // Display city name
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(x, y);
    display.print(cityName.c_str());
    y += 40;

    // Display temperature
    display.setFont(&FreeMonoBold12pt7b);
    String tempStr = String(temperature, 1) + " C";
    display.getTextBounds(tempStr.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = ((display.width() - tbw) / 2) - tbx;
    display.setCursor(x, y);
    display.print(tempStr.c_str());
    y += 25;

    // Display weather description
    display.getTextBounds(description.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = ((display.width() - tbw) / 2) - tbx;
    display.setCursor(x, y);
    display.print(description.c_str());
    y += 35;

    // Display current time
    display.setFont(&FreeMonoBold9pt7b);
    String timeStr = "Time: " + formatTime(currentTime);
    display.getTextBounds(timeStr.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = ((display.width() - tbw) / 2) - tbx;
    display.setCursor(x, y);
    display.print(timeStr.c_str());
    y += 20;

    // Display time difference since stored time
    String diffStr = "Since start: " + formatTimeDifference(timeDiff);
    display.getTextBounds(diffStr.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = ((display.width() - tbw) / 2) - tbx;
    display.setCursor(x, y);
    display.print(diffStr.c_str());
  }
  while (display.nextPage());
}

void loop() {
  // Example of time operations in the main loop
  delay(30000); // Wait 30 seconds
  
  if (timeInitialized) {
    TimeInfo currentTime = getCurrentTime();
    TimeInfo storedTime = getStoredTime();
    int timeDifference = getTimeDifferenceMinutes(storedTime, currentTime);
    
    Serial.println("Current time: " + formatTime(currentTime));
    Serial.println("Time since start: " + formatTimeDifference(timeDifference));
    
    // Example: Store a new reference time every 5 minutes
    if (timeDifference >= 5) {
      Serial.println("Storing new reference time...");
      storeCurrentTime();
    }
  }
}