/* File: APIUtils.h
   Purpose: Declares functions and structures for fetching and processing bus arrival data 
            from the Rejseplanen API for Arduino Nano ESP32. This header file provides
            a clean interface for bus data retrieval and display integration.
   Author: Jonas Kjeldmand Jensen
   Date: May 30, 2025
*/

#ifndef API_UTILS_H
#define API_UTILS_H

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                      REQUIRED INCLUDES                                                           ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║                                                                                                                   ║
// ║  Essential libraries needed for API functionality:                                                                ║
// ║  • GxEPD2_BW.h    - E-paper display control for showing bus arrival information                                   ║
// ║  • TimeUtils.h    - Time data structures and NTP synchronization functions                                       ║
// ║  • ArduinoJson.h  - JSON parsing for processing API responses from Rejseplanen                                   ║
// ║                                                                                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

#include <GxEPD2_BW.h>
#include <ArduinoJson.h>
#include "TimeUtils.h"

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                    REJSEPLANEN API CONFIGURATION                                                  ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║                                                                                                                   ║
// ║  API Credentials and Endpoint Information:                                                                        ║
// ║  • API Key: Personal access key for Rejseplanen API (required for all requests)                                  ║
// ║  • Stop IDs: Unique identifiers for the two bus stops we're monitoring                                           ║
// ║    - 1550: Gammel Kongevej (H.C. Ørsteds Vej) - Stop heading towards city center                               ║
// ║    - 1583: Gammel Kongevej (Alhambravej) - Stop heading towards outer areas                                     ║
// ║  • Base URL: https://www.rejseplanen.dk/api/                                                 ║
// ║                                                                                                                   ║
// ║  API Response Format:                                                                                             ║
// ║  The API returns JSON data containing arrival information for multiple bus lines.                                 ║
// ║  We filter for "Bus 1A" arrivals only and extract:                                                               ║
// ║  - Scheduled arrival time and date                                                                                ║
// ║  - Real-time arrival data (if available)                                                                         ║
// ║  - Stop identification                                                                                            ║
// ║  - Line name and route information                                                                                ║
// ║                                                                                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

// API credentials and configuration constants
extern const char* API_KEY;                    // Personal API key for Rejseplanen access
extern const char* MONITORED_STOP_IDS;         // Pipe-separated list of stop IDs to monitor
extern const char* TARGET_BUS_LINE;            // Specific bus line to track (e.g., "Bus 1A")

// Display-friendly stop names for the UI
extern const char* STOP_NAME_1550;             // Human-readable name for stop 1550
extern const char* STOP_NAME_1583;             // Human-readable name for stop 1583

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                      MEMORY MANAGEMENT                                                            ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║                                                                                                                   ║
// ║  JSON Processing Memory Allocation:                                                                               ║
// ║  The ESP32 has limited RAM, so we use StaticJsonDocument with carefully chosen sizes:                            ║
// ║                                                                                                                   ║
// ║  • JSON_FILTER_SIZE (256 bytes): Small filter document for specifying which JSON fields to parse                ║
// ║    This saves memory by ignoring unnecessary data from the API response                                           ║
// ║                                                                                                                   ║
// ║  • JSON_DOCUMENT_SIZE (4096 bytes): Main document for storing parsed API response                                ║
// ║    - 4096 bytes handles ~10-15 bus arrivals comfortably                                                          ║
// ║    - Reduces to 2048 bytes if memory is tight (handles ~5-8 arrivals)                                            ║
// ║    - Further reduces to 1024 bytes for extreme memory constraints (handles ~2-4 arrivals)                       ║
// ║                                                                                                                   ║
// ║  Memory Usage Guidelines:                                                                                         ║
// ║  - Standard operation: Use 4096 bytes for reliability                                                             ║
// ║  - Memory-constrained: Use 2048 bytes (monitor for parsing errors)                                               ║
// ║  - Emergency fallback: Use 1024 bytes (expect occasional failures during peak hours)                             ║
// ║                                                                                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

// JSON memory allocation constants
const size_t JSON_FILTER_SIZE = 256;           // Bytes for JSON filter document
const size_t JSON_DOCUMENT_SIZE = 4096;        // Bytes for main JSON parsing document

// Time calculation constants
const int MINUTES_PER_DAY = 1440;              // 24 hours * 60 minutes
const int MAX_LOOKAHEAD_MINUTES = 60;          // Only show arrivals within next hour
const int MAX_LOOKAHEAD_DAYS = 2;              // Search up to 2 days ahead

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                    FUNCTION DECLARATIONS                                                          ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║                                                                                                                   ║
// ║  Core API Functions:                                                                                              ║
// ║                                                                                                                   ║
// ║  setupAPIFilter()                                                                                                 ║
// ║  ├─ Purpose: Configures JSON filter to parse only needed fields from API response                               ║
// ║  ├─ When to call: Once during setup() before making any API requests                                             ║
// ║  ├─ Memory impact: Initializes 256-byte filter document                                                          ║
// ║  └─ Performance: Reduces JSON parsing time by 60-80% by ignoring unused fields                                   ║
// ║                                                                                                                   ║
// ║  checkBusArrivals()                                                                                               ║
// ║  ├─ Purpose: Main function that fetches, processes, and displays bus arrival data                                ║
// ║  ├─ When to call: Every minute in main loop (when minute changes)                                                ║
// ║  ├─ Parameters:                                                                                                   ║
// ║  │  ├─ currentTime: Current time from NTP server (TimeData structure)                                            ║
// ║  │  └─ display: Reference to e-paper display object for rendering results                                        ║
// ║  ├─ Network activity: Makes HTTPS request to Rejseplanen API                                                     ║
// ║  ├─ Memory usage: Allocates 4KB for JSON parsing (freed automatically when function ends)                       ║
// ║  ├─ Error handling: Gracefully handles network failures, JSON parsing errors, and invalid time                  ║
// ║  └─ Display update: Calls drawBusStopDisplay() to refresh e-paper screen with new data                           ║
// ║                                                                                                                   ║
// ║  Function Flow:                                                                                                   ║
// ║  1. Validate current time from NTP                                                                                ║
// ║  2. Build API request URL with current date/time                                                                 ║
// ║  3. Make HTTP GET request to Rejseplanen API                                                                      ║
// ║  4. Parse JSON response using pre-configured filter                                                               ║
// ║  5. Process arrivals: find next Bus 1A arrival for each stop                                                     ║
// ║  6. Calculate time differences and format display text                                                            ║
// ║  7. Update e-paper display with new information                                                                   ║
// ║  8. Handle errors gracefully (keep previous display content on failure)                                          ║
// ║                                                                                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

/**
 * @brief Initialize the JSON filter for efficient API response parsing
 * 
 * Sets up a filter that tells the JSON parser to only extract specific fields
 * from the Rejseplanen API response, significantly reducing memory usage and
 * parsing time. This filter focuses on arrival data while ignoring route
 * details, operator information, and other unnecessary fields.
 * 
 * Fields extracted by the filter:
 * - stopExtId: Unique identifier for the bus stop
 * - name: Bus line name (e.g., "Bus 1A")
 * - time: Scheduled arrival time (HH:MM format)
 * - date: Scheduled arrival date (YYYY-MM-DD format)
 * - rtTime: Real-time arrival time (HH:MM format, if available)
 * - rtDate: Real-time arrival date (YYYY-MM-DD format, if available)
 * 
 * @note This function must be called once during setup before making any API requests
 * @note The filter is stored in a global StaticJsonDocument for memory efficiency
 */
void setupAPIFilter();

/**
 * @brief Fetch bus arrivals from API and update the e-paper display
 * 
 * This is the main function that orchestrates the entire bus arrival checking process.
 * It handles network communication, data processing, error recovery, and display updates
 * in a single coordinated operation.
 * 
 * Process flow:
 * 1. Validates the provided current time (returns early if invalid)
 * 2. Constructs API request URL with current date and time parameters
 * 3. Makes HTTPS request to Rejseplanen multiArrivalBoard endpoint
 * 4. Parses JSON response using the pre-configured filter for efficiency
 * 5. Processes all arrivals to find the next Bus 1A for each monitored stop
 * 6. Calculates arrival time differences and formats user-friendly text
 * 7. Updates the e-paper display with the processed information
 * 8. Handles various error conditions gracefully without crashing
 * 
 * Error handling:
 * - Network connectivity issues: Maintains previous display content
 * - JSON parsing failures: Logs error and retains last known good data
 * - Invalid time data: Skips update cycle and waits for valid time
 * - API rate limiting: Respects HTTP error codes and avoids retry storms
 * 
 * Performance characteristics:
 * - HTTP request: Typically 2-5 seconds depending on network conditions
 * - JSON parsing: 100-300ms using filtered parsing (vs 1-2s unfiltered)
 * - Display update: 8-15 seconds for full e-paper refresh
 * - Total function time: Usually 10-20 seconds per call
 * 
 * Memory usage:
 * - Allocates 4KB for JSON document (automatically freed when function ends)
 * - Uses ~500 bytes for string manipulation during processing
 * - Peak memory usage: ~4.5KB during JSON parsing phase
 * 
 * @param currentTime Current time obtained from NTP server synchronization
 * @param display Reference to the initialized e-paper display object
 * 
 * @note This function should be called once per minute when the minute changes
 * @note Network connectivity is required - function handles offline conditions gracefully
 * @note Display updates are atomic - either fully successful or previous content remains
 * 
 * @warning Function blocks during HTTP request and display update (10-20 seconds total)
 * @warning Requires active WiFi connection to Rejseplanen API servers
 */
void checkBusArrivals(TimeData currentTime, GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>& display);

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                     DEBUGGING AND DIAGNOSTICS                                                    ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║                                                                                                                   ║
// ║  Debug Output Control:                                                                                            ║
// ║  The APIUtils module includes extensive debug output that can be enabled/disabled via                            ║
// ║  uncommented Serial.print() statements throughout the code. Debug categories include:                            ║
// ║                                                                                                                   ║
// ║  • API Request Debug: Shows complete request URLs, WiFi signal strength                                          ║
// ║  • HTTP Response Debug: Displays response codes, payload sizes, JSON snippets                                    ║
// ║  • JSON Parsing Debug: Reports parsing errors, arrival counts, memory usage                                      ║
// ║  • Bus Processing Debug: Shows arrival calculations, time differences, filtering                                  ║
// ║  • Display Output Debug: Previews formatted text before sending to e-paper display                              ║
// ║                                                                                                                   ║
// ║  To enable debugging:                                                                                             ║
// ║  1. Ensure Serial.begin(115200) is called in setup()                                                             ║
// ║  2. Uncomment desired debug blocks in APIUtils.cpp                                                                ║
// ║  3. Open Serial Monitor in Arduino IDE (Tools > Serial Monitor)                                                  ║
// ║  4. Set baud rate to 115200                                                                                      ║
// ║                                                                                                                   ║
// ║  Performance Impact of Debugging:                                                                                 ║
// ║  • Serial output adds 100-500ms per API call                                                                     ║
// ║  • Memory usage increases by ~200 bytes for debug strings                                                        ║
// ║  • Network timing may be affected by Serial.print() delays                                                       ║
// ║                                                                                                                   ║
// ║  Production Deployment:                                                                                           ║
// ║  • Comment out all debug Serial.print() statements for final deployment                                          ║
// ║  • This reduces memory usage and improves performance                                                             ║
// ║  • Critical errors are still handled gracefully without debug output                                             ║
// ║                                                                                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

#endif // API_UTILS_H