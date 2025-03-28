#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

// Load in other header files from the project folder
#include "DisplayConfig.h"
#include "Credentials.h"

// ESP8266 Feather HUZZAH Pin Connections for 4.2" EPD Module:
// 
// EPD Module Pin -> Feather HUZZAH Pin (Function)
// ----------------------------------------------
// BUSY  -> GPIO #16 (BUSY)
// RESET -> GPIO #5  (RES/RST)
// DC    -> GPIO #4  (DC)
// CS    -> GPIO #15 (CS/SS)
// SCL   -> GPIO #14 (SCK/SCL)
// SDA   -> GPIO #13 (MOSI/SDA)
// GND   -> GND      (Ground)
// VCC   -> 3V       (3.3V Power)

// Define the display HERE in the main sketch
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
  GxEPD2_420_GDEY042T81(/*CS=*/ 15, /*DC=*/ 4, /*RES=*/ 5, /*BUSY=*/ 16)
); // 400x300, SSD1683

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
    // Use WiFiClientSecure for HTTPS
    WiFiClientSecure client;
    client.setInsecure(); // Note: This bypasses SSL certificate verification

    HTTPClient https;
    String apiUrl = "https://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&appid=" + String(apiKey) + "&units=metric";
    
    if (https.begin(client, apiUrl)) {
      int httpResponseCode = https.GET();

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        String payload = https.getString();
        Serial.println("Received payload: " + payload);

        // Parse the JSON data
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          Serial.print("JSON parsing failed: ");
          Serial.println(error.c_str());
          return;
        }

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

      https.end();
    } else {
      Serial.println("Unable to connect to HTTPS server");
    }
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void loop() {
  // The main loop is empty as all the work is done in setup()
}