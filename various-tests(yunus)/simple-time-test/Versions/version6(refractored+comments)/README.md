# Bus Arrival Display System

A real-time bus arrival display system using ESP32 and e-paper display, fetching data from the Danish Rejseplanen API.

## Why Split Code Into Multiple Files?

### Benefits of Modular Code Structure

**Maintainability**: When code is organized into logical modules, finding and fixing bugs becomes much easier. Instead of scrolling through 400+ lines to find display-related code, you can go directly to `display.cpp`.

**Reusability**: Well-structured modules can be reused in other projects. The WiFi connection module, for example, could be used in any ESP32 project that needs internet connectivity.

**Collaboration**: Multiple developers can work on different modules simultaneously without conflicts. One person can work on the API integration while another improves the display logic.

**Testing**: Individual modules can be tested independently, making it easier to isolate problems and ensure each component works correctly.

**Readability**: Smaller, focused files are easier to understand. Each file has a single responsibility, making the codebase more approachable for new developers.

**Scalability**: Adding new features becomes simpler when you have a clear structure. Want to add a new API endpoint? Just extend the `api_client` module.

## System Architecture Overview

This project fetches real-time bus arrival data and displays it on an e-paper screen. Here's how the components work together:

```
┌─────────────────┐    ┌──────────────┐    ┌─────────────────┐
│   Main Loop     │───▶│ Time Manager │───▶│ API Client      │
│ (main.cpp)      │    │ (time.cpp)   │    │ (api_client.cpp)│
└─────────────────┘    └──────────────┘    └─────────────────┘
         │                      │                     │
         │                      │                     ▼
         │                      │            ┌─────────────────┐
         │                      │            │ Rejseplanen API │
         │                      │            │ (External)      │
         │                      │            └─────────────────┘
         │                      │
         ▼                      ▼
┌─────────────────┐    ┌──────────────┐
│ Display Manager │    │ WiFi Manager │
│ (display.cpp)   │    │ (wifi.cpp)   │
└─────────────────┘    └──────────────┘
         │                      │
         ▼                      ▼
┌─────────────────┐    ┌──────────────┐
│ E-Paper Screen  │    │ WiFi Network │
│ (Hardware)      │    │ (External)   │
└─────────────────┘    └──────────────┘
```

## File Structure and Responsibilities

### Core Files

#### `main.cpp` - System Orchestrator
**Purpose**: Main program loop and system initialization
**Key Functions**:
- `setup()`: Initialize all subsystems
- `loop()`: Main execution cycle, coordinates all modules
- System-wide error handling and state management

**Logic**: Acts as the conductor of the orchestra, ensuring all components work together harmoniously. Manages the overall program flow and timing.

#### `config.h` - Configuration Hub
**Purpose**: Centralized configuration and constants
**Contains**:
- WiFi credentials
- API keys and endpoints
- Pin definitions for hardware
- Display settings and fonts
- Timing intervals

**Logic**: Single source of truth for all configuration. Makes it easy to modify settings without hunting through multiple files.

### Network Layer

#### `wifi_manager.h/.cpp` - Connectivity Guardian
**Purpose**: Handles all WiFi-related operations
**Key Functions**:
- `initializeWiFi()`: Initial connection setup
- `maintainConnection()`: Automatic reconnection logic
- `getConnectionStatus()`: Current connection state
- `getIPAddress()`: Network information

**Logic**: Ensures reliable internet connectivity. Handles connection failures gracefully and provides status information to other modules.

#### `api_client.h/.cpp` - Data Fetcher
**Purpose**: Communicates with Rejseplanen API
**Key Functions**:
- `fetchBusArrivals()`: Get arrival data for specified stops
- `parseJsonResponse()`: Convert API response to usable data
- `calculateTimeDifferences()`: Compute minutes until arrival
- `filterRelevantArrivals()`: Extract Bus 1A arrivals only

**Logic**: Handles all external API communication. Processes raw JSON data into structured information about bus arrivals, including real-time vs scheduled data.

### Time Management

#### `time_manager.h/.cpp` - Time Keeper
**Purpose**: Manages time synchronization and calculations
**Key Functions**:
- `initializeNTP()`: Set up network time synchronization
- `getCurrentTime()`: Get current local time
- `syncWithNTP()`: Refresh time from internet
- `isTimeValid()`: Verify time data integrity

**Logic**: Ensures accurate time keeping for API queries and display. Handles time zone conversion and provides time in formats needed by other modules.

### Display Layer

#### `display_manager.h/.cpp` - Visual Interface
**Purpose**: Controls the e-paper display
**Key Functions**:
- `initializeDisplay()`: Set up e-paper hardware
- `updateBusInfo()`: Show bus arrival information
- `displayError()`: Show error messages
- `formatDisplayText()`: Prepare text for optimal display

**Logic**: Translates data into visual information. Manages screen layout, font selection, and ensures readable presentation of bus information.

### Data Structures

#### `bus_data.h` - Data Definitions
**Purpose**: Define data structures used throughout the system
**Contains**:
- `BusArrival` struct: Individual bus arrival information
- `StopInfo` struct: Bus stop details and status
- `TimeData` struct: Time representation
- Enums for system states and error codes

**Logic**: Provides consistent data types across all modules. Ensures all parts of the system speak the same "language" when sharing information.

## Data Flow Explanation

### 1. System Startup
```
Power On → Initialize Hardware → Connect WiFi → Sync Time → Ready State
```

### 2. Main Operation Cycle
```
Check Time → If New Minute → Fetch API Data → Process Results → Update Display → Wait
```

### 3. Error Handling Flow
```
Detect Error → Log Issue → Display Error Message → Attempt Recovery → Continue or Restart
```

## Module Interactions

### During Normal Operation:
1. **Main Loop** checks if a new minute has started using **Time Manager**
2. **Main Loop** requests **WiFi Manager** to ensure connectivity
3. **Main Loop** asks **API Client** to fetch fresh bus data
4. **API Client** uses **WiFi Manager** for HTTP requests
5. **API Client** processes JSON and returns structured data
6. **Main Loop** passes processed data to **Display Manager**
7. **Display Manager** formats and shows information on screen

### During Error Conditions:
- **WiFi Manager** automatically attempts reconnection
- **Time Manager** handles NTP sync failures gracefully  
- **API Client** provides fallback data during network issues
- **Display Manager** shows appropriate error messages
- **Main Loop** coordinates recovery attempts

## Hardware Connections

### E-Paper Display (GDEY042T81 - 4.2" 400x300px)
```
ESP32 Pin → E-Paper Pin
D10 (CS)  → CS (Chip Select)
D9  (DC)  → DC (Data/Command)
D8  (RST) → RST (Reset)
D7  (BUSY)→ BUSY (Busy Status)
D12 (SCK) → CLK (Serial Clock)
D11 (MOSI)→ DIN (Data In)
3V3       → VCC (Power)
GND       → GND (Ground)
```

## Configuration Requirements

### WiFi Settings (config.h)
```cpp
const char* WIFI_SSID = "YourNetworkName";
const char* WIFI_PASSWORD = "YourPassword";
```

### API Configuration
```cpp
const char* API_KEY = "your-rejseplanen-api-key";
const char* STOP_IDS = "1550|1583";  // Pipe-separated stop IDs
```

### Time Zone Settings
```cpp
const long GMT_OFFSET_SEC = 7200;     // UTC+2 for Copenhagen
const int DAYLIGHT_OFFSET_SEC = 0;    // Handled by NTP
```

## System States and Behaviors

### Normal Operation
- Updates display every minute
- Shows next Bus 1A arrival for both stops
- Indicates real-time (RT) vs scheduled (SCH) data
- Displays current time and last update timestamp

### Error States
- **WiFi Error**: Shows connection status, attempts reconnection
- **Time Error**: Indicates NTP sync issues, continues with cached time
- **API Error**: Shows HTTP/JSON errors, retries on next cycle
- **Display Error**: Logs issues but continues operation

## Power Management
The system is designed for continuous operation but includes:
- Minimal display updates (only when data changes)
- Efficient HTTP client usage (connection reuse disabled for stability)
- Low-power e-paper display (only draws power during updates)

## Troubleshooting Guide

### Common Issues:
1. **Display not updating**: Check SPI connections and power supply
2. **WiFi connection fails**: Verify credentials and signal strength  
3. **No bus data**: Confirm API key validity and stop IDs
4. **Incorrect time**: Check NTP server accessibility and time zone settings
5. **System resets**: Monitor serial output for error patterns

This modular structure makes the system robust, maintainable, and extensible while keeping each component focused on its specific responsibility.