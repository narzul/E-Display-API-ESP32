#include "api_handler.h"

APIHandler::APIHandler() {}

void APIHandler::begin(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

String APIHandler::fetchData(const char* url, const char* headerName, const char* headerValue) {
    String payload = "";
    if (isWiFiConnected()) {
        HTTPClient http;
        http.begin(url);
        if (headerName && headerValue) {
            http.addHeader(headerName, headerValue);
        }
        int httpCode = http.GET();
        if (httpCode > 0) {
            payload = http.getString();
        } else {
            payload = "Error on HTTP request";
        }
        http.end();
    } else {
        payload = "WiFi not connected";
    }
    return payload;
}

String APIHandler::parseJSON(const String& jsonString, const char* key) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, jsonString);
    return doc[key].as<String>();
}

bool APIHandler::isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}