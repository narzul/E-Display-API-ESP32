/*
* ESP32-Thing-4.2E-PaperDisplay+API.ino
* Made by Jonas Kjeldmnd Jensen (Yunus), September 2024
* Extended with comprehensive time management functionality
*
* Main Arduino file that coordinates weather display with smart time management
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

// Load project header files
#include "DisplayConfig.h"
#include "Credentials.h"
#include "TimeManager.h"

// =========================
// CONFIGURATION
// =========================

const char* city = "Copenhagen";  // Weather location

// =========================
// SETUP FUNCTION
// =========================

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== ESP32 Weather Display with Smart Time Management ===");
  Serial.println("Made by Jonas Kjeldmnd Jensen (Yunus), September 2024");
  Serial.println("========================================================");
  
  // Initialize display
  Serial.println("🖥️  Initializing e-paper display...");
  display.init(115200, true, 50, false);
  display.setRotation(4); // 90 degrees counter-clockwise
  Serial.println("✓ E-paper display initialized");
  
  // Connect to WiFi
  connectToWifi();
  
  // Initialize time management
  if (initializeTime()) {
    storeCurrentTime();
    Serial.println("✓ Time management system ready");
  } else {
    Serial.println("✗ Time initialization failed - will retry in loop");
  }
  
  // Initialize weather data structure
  currentWeather.valid = false;
  
  // Fetch initial weather data
  fetchWeatherData();
  
  // Initial display update
  updateDisplay();
  
  Serial.println("\n=== Setup Complete - Entering Main Loop ===\n");
}

// =========================
// WIFI FUNCTIONS
// =========================

void connectToWifi() {
  Serial.print("📶 Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" Connected!");
    Serial.println("IP address: " + WiFi.localIP().toString());
    Serial.println("Signal strength: " + String(WiFi.RSSI()) + " dBm");
  } else {
    Serial.println(" Failed to connect!");
    Serial.println("⚠️  Will continue without WiFi - limited functionality");
  }
}

// =========================
// WEATHER FUNCTIONS
// =========================

void fetchWeatherData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("📶 WiFi not connected, skipping weather update");
    return;
  }
  
  Serial.println("🌤️  Fetching weather data for " + String(city) + "...");
  
  HTTPClient http;
  String apiUrl = "https://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&appid=" + String(apiKey) + "&units=metric";
  http.begin(apiUrl);
  http.setTimeout(10000); // 10 second timeout
  
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.println("Weather API Response: " + String(httpResponseCode));
    
    if (httpResponseCode == 200) {
      String payload = http.getString();
      
      // Parse JSON response
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (error) {
        Serial.println("✗ JSON parsing failed: " + String(error.c_str()));
        currentWeather.valid = false;
      } else {
        // Extract weather data
        currentWeather.cityName = doc["name"].as<String>();
        currentWeather.temperature = doc["main"]["temp"].as<float>();
        currentWeather.description = doc["weather"][0]["description"].as<String>();
        currentWeather.fetchTime = getCurrentTime();
        currentWeather.valid = true;
        
        // Update timing
        lastWeatherUpdate = millis();
        
        Serial.println("✓ Weather data updated:");
        Serial.println("  🏙️  City: " + currentWeather.cityName);
        Serial.println("  🌡️  Temperature: " + String(currentWeather.temperature, 1) + "°C");
        Serial.println("  ☁️  Conditions: " + currentWeather.description);
        Serial.println("  🕐 Fetched at: " + formatTime(currentWeather.fetchTime));
      }
    } else {
      Serial.println("✗ Weather API returned error code: " + String(httpResponseCode));
      currentWeather.valid = false;
    }
  } else {
    Serial.println("✗ Weather API connection failed: " + String(httpResponseCode));
    currentWeather.valid = false;
  }
  
  http.end();
}

// =========================
// DISPLAY FUNCTIONS
// =========================

void updateDisplay() {
  Serial.println("🖥️  Updating e-paper display...");
  
  TimeInfo currentTime = getCurrentTime();
  int timeSinceStart = getTimeDifferenceMinutes(storedTime, currentTime);
  int timeSinceWeather = currentWeather.valid ? getTimeDifferenceMinutes(currentWeather.fetchTime, currentTime) : 0;
  
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    
    int16_t tbx, tby; 
    uint16_t tbw, tbh;
    uint16_t y = 50;
    
    if (currentWeather.valid) {
      // City name (large font)
      display.setFont(&FreeMonoBold24pt7b);
      display.getTextBounds(currentWeather.cityName.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
      uint16_t x = ((display.width() - tbw) / 2) - tbx;
      display.setCursor(x, y);
      display.print(currentWeather.cityName.c_str());
      y += 45;
      
      // Temperature (medium font)
      display.setFont(&FreeMonoBold12pt7b);
      String tempStr = String(currentWeather.temperature, 1) + " C";
      display.getTextBounds(tempStr.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
      x = ((display.width() - tbw) / 2) - tbx;
      display.setCursor(x, y);
      display.print(tempStr.c_str());
      y += 30;
      
      // Weather description
      display.getTextBounds(currentWeather.description.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
      x = ((display.width() - tbw) / 2) - tbx;
      display.setCursor(x, y);
      display.print(currentWeather.description.c_str());
      y += 40;
    } else {
      // No weather data available
      display.setFont(&FreeMonoBold12pt7b);
      String errorMsg = "Weather Unavailable";
      display.getTextBounds(errorMsg.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
      uint16_t x = ((display.width() - tbw) / 2) - tbx;
      display.setCursor(x, y);
      display.print(errorMsg.c_str());
      y += 60;
    }
    
    // Time information (small font)
    display.setFont(&FreeMonoBold9pt7b);
    
    // Current time
    String timeStr = "Time: " + formatTimeShort(currentTime);
    display.getTextBounds(timeStr.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    display.setCursor(x, y);
    display.print(timeStr.c_str());
    y += 22;
    
    // System uptime
    String uptimeStr = "Uptime: " + formatTimeDifference(timeSinceStart);
    display.getTextBounds(uptimeStr.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    x = ((display.width() - tbw) / 2) - tbx;
    display.setCursor(x, y);
    display.print(uptimeStr.c_str());
    y += 22;
    
    // Weather data age
    if (currentWeather.valid) {
      String weatherAgeStr = "Weather: " + formatTimeDifference(timeSinceWeather) + " ago";
      display.getTextBounds(weatherAgeStr.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
      x = ((display.width() - tbw) / 2) - tbx;
      display.setCursor(x, y);
      display.print(weatherAgeStr.c_str());
    }
    
  } while (display.nextPage());
  
  lastDisplayUpdate = millis();
  Serial.println("✓ Display updated successfully");
}

// =========================
// MAIN LOOP
// =========================

void loop() {
  // Handle time initialization retry
  if (!timeInitialized) {
    Serial.println("🕐 Time not initialized, retrying...");
    if (initializeTime()) {
      storeCurrentTime();
    } else {
      delay(5000); // Wait 5 seconds before retry
      return;
    }
  }
  
  // Check for automatic NTP re-sync (every 12 hours)
  checkForNtpSync();
  
  // Update weather data if needed (every 10 minutes)
  if (shouldUpdateWeather()) {
    fetchWeatherData();
  }
  
  // Update display if needed (every 30 seconds)  
  if (shouldUpdateDisplay()) {
    updateDisplay();
  }
  
  // Print detailed status to Serial Monitor every 15 seconds
  static unsigned long lastSerialUpdate = 0;
  if (millis() - lastSerialUpdate > 15000) {
    TimeInfo currentTime = getCurrentTime();
    int uptimeMinutes = getTimeDifferenceMinutes(storedTime, currentTime);
    
    Serial.println("\n📊 === System Status ===");
    Serial.println("🕐 Current time: " + formatTime(currentTime));
    Serial.println("⏱️  System uptime: " + formatTimeDifference(uptimeMinutes));
    Serial.println("📶 WiFi status: " + String(WiFi.status() == WL_CONNECTED ? "Connected (" + String(WiFi.RSSI()) + " dBm)" : "Disconnected"));
    
    if (currentWeather.valid) {
      int weatherAge = getTimeDifferenceMinutes(currentWeather.fetchTime, currentTime);
      Serial.println("🌤️  Weather: " + currentWeather.cityName + ", " + 
                     String(currentWeather.temperature, 1) + "°C, " + 
                     currentWeather.description);
      Serial.println("📅 Weather age: " + formatTimeDifference(weatherAge));
    } else {
      Serial.println("🌤️  Weather: No valid data available");
    }
    
    Serial.println("💾 Free memory: " + String(ESP.getFreeHeap()) + " bytes");
    Serial.println("🔋 Uptime: " + String(getUptimeSeconds()) + " seconds");
    Serial.println("========================\n");
    
    lastSerialUpdate = millis();
  }
  
  // Small delay to prevent system overload
  delay(1000);
}