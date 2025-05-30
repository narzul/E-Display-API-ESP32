/* File: WiFiUtils.h
   Purpose: Header declarations for advanced WiFi management system.
            Provides enterprise-grade connection handling, monitoring,
            and diagnostics for Arduino Nano ESP32 projects.
   Author: Jonas Kjeldmand Jensen
   Date: May 30, 2025
*/

#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include <Arduino.h>

// ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
// ║                                    WIFI MANAGEMENT FUNCTIONS                                                      ║
// ╠═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
// ║                                                                                                                   ║
// ║  Core WiFi Functions:                                                                                             ║
// ║  • setupWiFi()          - Initialize WiFi connection with timeout protection                                      ║
// ║  • maintainWiFi()       - Intelligent reconnection with exponential backoff                                      ║
// ║  • printConnectionDetails() - Comprehensive connection diagnostics                                                ║
// ║                                                                                                                   ║
// ║  Monitoring Functions:                                                                                            ║
// ║  • isWiFiConnected()    - Quick connection status check                                                           ║
// ║  • getSignalStrength()  - Current RSSI value in dBm                                                              ║
// ║  • getConnectionStatus() - Human-readable connection status                                                       ║
// ║                                                                                                                   ║
// ║  Usage Pattern:                                                                                                   ║
// ║  1. Call setupWiFi() once in setup() function                                                                    ║
// ║  2. Call maintainWiFi() periodically in loop() or before network operations                                      ║
// ║  3. Use monitoring functions for system health checks and debugging                                               ║
// ║                                                                                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

// ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════
// PRIMARY WIFI MANAGEMENT FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════

/**
 * Initialize WiFi connection with comprehensive error handling
 * 
 * Features:
 * - Timeout protection (20 second maximum wait)
 * - Detailed connection diagnostics
 * - Signal quality assessment
 * - Non-blocking operation (continues even if initial connection fails)
 * 
 * Call this once in your setup() function.
 */
void setupWiFi();

/**
 * Maintain WiFi connection with intelligent reconnection
 * 
 * Features:
 * - Automatic disconnect detection
 * - Multiple reconnection attempts with exponential backoff
 * - Comprehensive error reporting
 * - Graceful failure handling
 * 
 * Call this periodically (e.g., before API calls or in main loop).
 * It's lightweight when connection is healthy - only acts when needed.
 */
void maintainWiFi();

/**
 * Print comprehensive WiFi connection details
 * 
 * Displays:
 * - Network information (SSID, IP, gateway, DNS)
 * - Signal strength with quality assessment
 * - MAC address for troubleshooting
 * - Performance recommendations for weak signals
 * 
 * Automatically called after successful connections.
 */
void printConnectionDetails();

// ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════
// WIFI MONITORING AND DIAGNOSTICS
// ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════

/**
 * Check if WiFi is currently connected
 * 
 * @return true if connected to WiFi network, false otherwise
 * 
 * Use this for quick connection status checks before network operations.
 * More efficient than checking WiFi.status() directly.
 */
bool isWiFiConnected();

/**
 * Get current WiFi signal strength
 * 
 * @return RSSI value in dBm, or -999 if not connected
 * 
 * Signal quality guide:
 * - Greater than -30 dBm: Excellent (very close to router)
 * - -30 to -50 dBm: Good (normal indoor range)
 * - -50 to -70 dBm: Fair (still usable)
 * - -70 to -80 dBm: Poor (may have issues)
 * - Less than -80 dBm: Very poor (expect problems)
 */
int getSignalStrength();

/**
 * Get human-readable WiFi connection status
 * 
 * @return String describing current connection state
 * 
 * Possible return values:
 * - "Connected" - Successfully connected to network
 * - "Network not found" - SSID not available
 * - "Connection failed" - Wrong password or network issue
 * - "Connection lost" - Was connected but lost connection
 * - "Disconnected" - Not connected
 * - "Unknown status" - Unexpected WiFi status
 */
String getConnectionStatus();

#endif // WIFI_UTILS_H