/* File: main.ino
   Purpose: Main entry point for the bus stop sign project using an Arduino Nano ESP32.
            Integrates a 4.2-inch e-paper display to show bus arrival times fetched from
            the Rejseplanen API, with time synchronized via NTP over WiFi.
   Author: Jonas Kjeldmand Jensen
   Date: May 30, 2025
*/

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                    ARDUINO NANO ESP32 PINOUT REFERENCE                                            ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║                                                                                                                   ║
// ║   USB-C Connector                          Arduino Nano ESP32 (Top View)                                          ║
// ║         ┌─────┐                                                                                                   ║
// ║         │     │                            D1  TX0 ●○● VIN     (VIN = 5V input)                                   ║
// ║         │ USB │                            D0  RX0 ●○● GND     (Ground)                                           ║
// ║         │  C  │                            RST     ●○● RST     (Reset)                                            ║
// ║         │     │                            GND     ●○● 5V      (5V output)                                        ║
// ║         └─────┘                            D2  IO2 ●○● A7      (GPIO2  - Safe for general use)                    ║
// ║                                            D3  IO3 ●○● A6      (GPIO3  - Safe for general use)                    ║
// ║           ┌──────────────────────────────────────────────────────────────┐                                        ║
// ║           │  D1●    D3●    D5●    D7●    D9●    D11●   D13●   RST●       │                                        ║
// ║           │  D0●    D2●    D4●    D6●    D8●    D10●   D12●   GND●       │                                        ║
// ║           └──────────────────────────────────────────────────────────────┘                                        ║
// ║                  D4  IO4 ●○● A5      (GPIO4  - Safe for general use)                                              ║
// ║                  D5  IO5 ●○● A4      (GPIO5  - Safe for general use)                                              ║
// ║                  D6  IO6 ●○● A3      (GPIO6  - Safe for general use)                                              ║
// ║                  D7  IO7 ●○● A2      (GPIO7  - Safe for general use)                                              ║
// ║                  D8  IO8 ●○● A1      (GPIO8  - Safe for general use)                                              ║
// ║                  D9  IO9 ●○● A0      (GPIO9  - Safe for general use)                                              ║
// ║                  D10 IO10●○● AREF    (GPIO10 - Safe for general use)                                              ║
// ║                  D11 IO21●○● 3V3     (GPIO21 - Safe for general use, MOSI/SDA)                                    ║
// ║                  D12 IO20●○● D13     (GPIO20 - Safe for general use, MISO/SCL, GPIO18 - SCK)                      ║
// ║                                                                                                                   ║
// ║  HARDWARE SPI PINS (Fixed - Don't Change):                                                                        ║
// ║  • SCK  (Serial Clock)     = D13 = GPIO18  ← Must use for SPI                                                     ║
// ║  • MOSI (Master Out)       = D11 = GPIO21  ← Must use for SPI                                                     ║
// ║  • MISO (Master In)        = D12 = GPIO20  ← Not used for e-paper                                                 ║
// ║                                                                                                                   ║
// ║  RECOMMENDED E-PAPER CONTROL PINS:                                                                                ║
// ║  • CS   (Chip Select)      = D10 = GPIO10  ← Controls when display listens                                        ║
// ║  • DC   (Data/Command)     = D9  = GPIO9   ← Tells display if data is command or pixel data                       ║
// ║  • RST  (Reset)            = D8  = GPIO8   ← Resets the display                                                   ║
// ║  • BUSY (Busy Signal)      = D7  = GPIO7   ← Display tells us when it's updating                                  ║
// ║                                                                                                                   ║
// ║  POWER CONNECTIONS:                                                                                               ║
// ║  • VCC  → 3.3V (3V3 pin)   ← E-paper displays typically use 3.3V                                                  ║
// ║  • GND  → GND               ← Common ground                                                                       ║
// ║                                                                                                                   ║
// ║  WIRING SUMMARY FOR 4.2" E-PAPER DISPLAY:                                                                         ║
// ║  ┌─────────────────┬─────────────────┬─────────────────┬─────────────────────────────────────────────────────┐    ║
// ║  │ E-Paper Pin     │ Arduino Pin     │ GPIO Number     │ Function                                            │    ║
// ║  ├─────────────────┼─────────────────┼─────────────────┼─────────────────────────────────────────────────────┤    ║
// ║  │ VCC             │ 3V3             │ Power           │ 3.3V power supply                                   │    ║
// ║  │ GND             │ GND             │ Ground          │ Common ground reference                             │    ║
// ║  │ DIN (MOSI)      │ D11             │ GPIO21          │ Data input - pixel data from ESP32 to display       │    ║
// ║  │ CLK (SCK)       │ D13             │ GPIO18          │ Clock signal for synchronizing data transfer        │    ║
// ║  │ CS              │ D10             │ GPIO10          │ Chip select - enables communication with display    │    ║
// ║  │ DC              │ D9              │ GPIO9           │ Data/Command - distinguishes commands from data     │    ║
// ║  │ RST (Reset)     │ D8              │ GPIO8           │ Reset pin - restarts display initialization         │    ║
// ║  │ BUSY            │ D7              │ GPIO7           │ Busy signal - display tells ESP32 when it's busy    │    ║
// ║  └─────────────────┴─────────────────┴─────────────────┴─────────────────────────────────────────────────────┘    ║
// ║                                                                                                                   ║
// ║  NOTE: These pins (GPIO7-10) are safe on Arduino Nano ESP32 and don't conflict with boot/flash pins               ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

// Include libraries for the e-paper display
#include <GxEPD2_BW.h>              // Library for controlling e-paper displays
#include <Fonts/FreeMonoBold9pt7b.h> // Small bold font for text
#include <Fonts/FreeMonoBold12pt7b.h> // Medium bold font for text
#include <Fonts/FreeMonoBold24pt7b.h> // Large bold font for time or headers

// Include custom module headers
#include "TimeUtils.h"   // For NTP time synchronization
#include "WiFiUtils.h"   // For WiFi connection management
#include "APIUtils.h"    // For fetching and processing API data
#include "DisplayUtils.h" // For managing the e-paper display

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                        DISPLAY CONFIGURATION                                                      ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║                                                                                                                   ║
// ║  Define the e-paper display object for a 4.2-inch display (400x300 resolution)                                  ║
// ║  Model: GDEY042T81 (black and white e-paper display)                                                             ║
// ║                                                                                                                   ║
// ║  Pin Configuration Explanation:                                                                                   ║
// ║  • CS=10   (GPIO10) - Chip Select: ESP32 pulls this LOW to talk to the display                                  ║
// ║  • DC=9    (GPIO9)  - Data/Command: LOW=command, HIGH=pixel data                                                 ║
// ║  • RST=8   (GPIO8)  - Reset: Pulse LOW to reset display (like pressing reset button)                           ║
// ║  • BUSY=7  (GPIO7)  - Busy: Display pulls HIGH when updating (ESP32 waits for LOW)                             ║
// ║                                                                                                                   ║
// ║  SPI pins are hardwired and automatically used:                                                                  ║
// ║  • SCK (Clock) = GPIO18 (D13) - Timing signal for data transfer                                                 ║
// ║  • MOSI (Data) = GPIO21 (D11) - Actual pixel/command data                                                       ║
// ║                                                                                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
  GxEPD2_420_GDEY042T81(
    /*CS=*/   10,  // D10 = GPIO10 - Chip Select (Safe pin on Nano ESP32)
    /*DC=*/   9,   // D9  = GPIO9  - Data/Command (Safe pin on Nano ESP32)  
    /*RST=*/  8,   // D8  = GPIO8  - Reset (Safe pin on Nano ESP32)
    /*BUSY=*/ 7    // D7  = GPIO7  - Busy Signal (Safe pin on Nano ESP32)
  )
);

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                     TROUBLESHOOTING GUIDE                                                         ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║                                                                                                                   ║
// ║  If you experience issues, try these alternative pin combinations:                                                ║
// ║                                                                                                                   ║
// ║  ALTERNATIVE 1 - If pins 7-10 don't work:                                                                        ║
// ║  GxEPD2_420_GDEY042T81(/*CS=*/ 5, /*DC=*/ 4, /*RST=*/ 2, /*BUSY=*/ 3)                                          ║
// ║  Wire: CS→D5, DC→D4, RST→D2, BUSY→D3                                                                             ║
// ║                                                                                                                   ║
// ║  ALTERNATIVE 2 - Conservative safe pins:                                                                          ║
// ║  GxEPD2_420_GDEY042T81(/*CS=*/ 6, /*DC=*/ 5, /*RST=*/ 4, /*BUSY=*/ 2)                                          ║
// ║  Wire: CS→D6, DC→D5, RST→D4, BUSY→D2                                                                             ║
// ║                                                                                                                   ║
// ║  ALWAYS KEEP THESE THE SAME (Hardware SPI):                                                                      ║
// ║  • SCK  = D13 (GPIO18)                                                                                           ║
// ║  • MOSI = D11 (GPIO21)                                                                                           ║
// ║  • VCC  = 3V3                                                                                                    ║
// ║  • GND  = GND                                                                                                    ║
// ║                                                                                                                   ║
// ║  UPLOAD TROUBLESHOOTING:                                                                                          ║
// ║  1. Hold BOOT button while clicking Upload in Arduino IDE                                                        ║
// ║  2. Use a good quality USB-C cable (data, not just charging)                                                     ║
// ║  3. Select correct board: "Arduino Nano ESP32" in Tools > Board                                                  ║
// ║  4. Select correct port in Tools > Port                                                                          ║
// ║  5. Set Upload Speed to 115200 if having issues                                                                  ║
// ║                                                                                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

// Setup function: Runs once when the Arduino starts
void setup() {
  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  // SERIAL DEBUGGING SETUP
  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  // Uncomment the next line to enable debug output via Serial Monitor
  // This is very helpful for troubleshooting WiFi, API, and display issues
  Serial.begin(115200);  
  
  // Give the serial connection time to initialize
  delay(1000);
  
  Serial.println("╔══════════════════════════════════════════════════════════════════════════╗");
  Serial.println("║                      ESP32 Bus Stop Display                             ║");
  Serial.println("║                         Starting Setup...                               ║");
  Serial.println("╚══════════════════════════════════════════════════════════════════════════╝");

  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  // DISPLAY INITIALIZATION
  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  Serial.println("🖥️  Initializing 4.2\" e-paper display...");
  Serial.println("   Display pins: CS=D10, DC=D9, RST=D8, BUSY=D7");
  Serial.println("   SPI pins: SCK=D13(GPIO18), MOSI=D11(GPIO21)");
  
  setupDisplay(display);
  
  Serial.println("✅ Display initialized successfully!");

  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  // WIFI CONNECTION
  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  Serial.println("📶 Connecting to WiFi...");
  
  setupWiFi();
  
  Serial.print("✅ WiFi connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("   Signal strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  // TIME SYNCHRONIZATION
  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  Serial.println("🕐 Synchronizing time with NTP server...");
  
  setupNTP();
  
  TimeData currentTime = getCurrentTimeFromNTP();
  if (currentTime.hours != -1) {
    Serial.print("✅ Time synchronized! Current time: ");
    printTime(currentTime);
  } else {
    Serial.println("⚠️  Warning: Time synchronization may have failed");
  }

  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  // API CONFIGURATION
  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  Serial.println("🚌 Setting up Rejseplanen API filters...");
  
  setupAPIFilter();
  
  Serial.println("✅ API configuration complete!");

  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  // SETUP COMPLETE
  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  Serial.println("╔══════════════════════════════════════════════════════════════════════════╗");
  Serial.println("║                        Setup Complete!                                  ║");
  Serial.println("║              Starting bus arrival monitoring...                         ║");
  Serial.println("║                                                                          ║");
  Serial.println("║  The display will update every minute with fresh bus arrival data.     ║");
  Serial.println("║  Watch this Serial Monitor for debug information and status updates.   ║");
  Serial.println("╚══════════════════════════════════════════════════════════════════════════╝");
  Serial.println();
}

// Loop function: Runs continuously after setup
void loop() {
  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  // MAIN MONITORING LOOP
  // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
  static int lastMinute = -1;  // Tracks the last minute processed to avoid duplicate updates
  static unsigned long lastStatusPrint = 0;  // For periodic status updates
  
  // Get current time from NTP server
  TimeData currentTime = getCurrentTimeFromNTP();
  
  // Check if we have valid time and if the minute has changed (update once per minute)
  if (currentTime.hours != -1 && currentTime.minutes != lastMinute) {
    lastMinute = currentTime.minutes;  // Remember this minute so we don't update again
    
    Serial.println("────────────────────────────────────────────────────────────────────────");
    Serial.print("🕐 Time update: ");
    printTime(currentTime);
    Serial.println("📡 Fetching fresh bus arrival data from Rejseplanen API...");
    
    // Maintain WiFi connection (reconnect if needed)
    maintainWiFi();
    
    // Fetch bus arrivals and update the display
    checkBusArrivals(currentTime, display);
    
    Serial.println("✅ Display updated with latest bus arrivals");
    Serial.println("⏱️  Next update in 1 minute...");
    Serial.println("────────────────────────────────────────────────────────────────────────");
    Serial.println();
  }
  
  // Print periodic status updates (every 30 seconds) to show the system is alive
  unsigned long currentMillis = millis();
  if (currentMillis - lastStatusPrint >= 30000) {  // 30 seconds
    lastStatusPrint = currentMillis;
    
    Serial.print("💓 System status: Running | WiFi: ");
    Serial.print(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    Serial.print(" | Signal: ");
    Serial.print(WiFi.RSSI());
    Serial.print(" dBm | Free memory: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
  }
  
  // Short delay to prevent overwhelming the system
  // Check time every 100ms (10 times per second) for responsive updates
  delay(100);
}