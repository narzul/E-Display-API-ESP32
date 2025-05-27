#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "Labitat (free)";
const char* password = "labitatisawesome";

// Replace with your actual API URL and accessId
const char* apiURL = "https://www.rejseplanen.dk/api/arrivalBoard?id=A%3D1%40O%3DGammel%20Kongevej%20(H.C.%20%D8rsteds%20Vej)%40X=12545525%40Y=55676021%40U=86%40L=1550%40date=2025-05-20&time=23:45&accessId=9b00b65e-e873-45af-8ff8-47366a137f53&format=json";

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to Wi-Fi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Make HTTP GET request
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(apiURL); // Specify the URL
    int httpResponseCode = http.GET(); // Make the request

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println("API Response:");
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end(); // Free resources
  } else {
    Serial.println("WiFi not connected");
  }
}

void loop() {
  // Empty loop
}
