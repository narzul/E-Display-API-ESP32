#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "credentials.h"

// Initialize the display for the 4.2" ePaper module
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(
  /*CS=5*/   5,    // Chip Select pin
  /*DC=*/    0,    // Data/Command pin
  /*RST=*/   2,    // Reset pin
  /*BUSY=*/  15    // Busy pin
  // SCL (Serial Clock) is connected to GPIO 18
  // SDA (Serial Data / MOSI) is connected to GPIO 23
// Note: SCL (GPIO 18) and SDA (GPIO 23) are automatically used by the ESP32's SPI interface
));

// OpenWeatherMap API details
const char* apiKey = "YOUR_OPENWEATHERMAP_API_KEY";
const char* city = "London";
const char* countryCode = "UK";

void setup()
{
  Serial.begin(115200);
  
  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize the display
  display.init(115200, true, 50, false);
  
  // Fetch and display weather data
  displayWeatherData();
  
  // Put the display in low power mode
  display.hibernate();
}

void displayWeatherData()
{
  String weatherData = getWeatherData();
  
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(10, 30);
    display.print("Weather in ");
    display.print(city);
    display.print(", ");
    display.println(countryCode);
    
    display.setCursor(10, 60);
    display.println(weatherData);
  }
  while (display.nextPage());
}

String getWeatherData()
{
  String result = "";
  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "," + String(countryCode) + "&appid=" + String(apiKey) + "&units=metric";
    
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String payload = http.getString();
      
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      
      float temp = doc["main"]["temp"];
      int humidity = doc["main"]["humidity"];
      String description = doc["weather"][0]["description"];
      
      result = "Temp: " + String(temp, 1) + "°C\n";
      result += "Humidity: " + String(humidity) + "%\n";
      result += "Desc: " + description;
    }
    else {
      result = "Error fetching data";
    }
    
    http.end();
  }
  else {
    result = "WiFi not connected";
  }
  
  return result;
}

void loop() {
  // The main loop is empty as all the work is done in setup()
}