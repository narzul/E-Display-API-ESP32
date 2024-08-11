#ifndef API_HANDLER_H
#define API_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class APIHandler {
public:
    APIHandler();
    void begin(const char* ssid, const char* password);
    String fetchData(const char* url, const char* headerName = nullptr, const char* headerValue = nullptr);
    String parseJSON(const String& jsonString, const char* key);

private:
    bool isWiFiConnected();
};

#endif