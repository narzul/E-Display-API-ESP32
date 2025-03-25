/*
*
* Made by Jonas Kjeldmnd Jensen (Yunus), September 2024
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

      // Display the data on the e-paper display
      display.setFont(&FreeMonoBold24pt7b);
      display.setTextColor(GxEPD_BLACK);

      int16_t tbx, tby; uint16_t tbw, tbh;
      display.getTextBounds(cityName.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
      uint16_t x = ((display.width() - tbw) / 2) - tbx;
      uint16_t y = ((display.height() - tbh) / 2) - tby;

      display.setFullWindow();
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y);
        display.print(cityName.c_str());

        y += tbh + 5; // Adjust y for the next line

        display.setFont(&FreeMonoBold12pt7b);  // Set font for the description
        display.setCursor(x, y);
        display.print(description.c_str());

        y += tbh - 15; // Adjust y for the next line

        display.setFont(&FreeMonoBold9pt7b);  // Set font for the temperature
        display.setCursor(x, y);  // Set cursor for temperature
        display.print(String(temperature, 1) + " C");
      }
      while (display.nextPage());
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void loop() {
  // The main loop is empty as all the work is done in setup()
}