# ESP8266 Weather Display

<div align="center" title="This is a working weather API fetcher showing the weather in Copenhagen." style="max-width: 75%; margin: 20px auto; padding: 10px; border: 2px solid #ccc; background-color: #f9f9f9; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); border-radius: 8px; width: 75%;">
  <img src="/src/assets/img/Weather-API-ESP8266.jpeg" alt="Image of a working weather API fetcher showing the weather in Copenhagen" style="width: 75%; height: auto; display: block; border-radius: 8px;">
</div>

This project creates a simple weather station using an ESP8266 Feather HUZZAH and a 4.2" e-paper display. It connects to the OpenWeatherMap API to fetch current weather data for a specified city and displays it on the e-paper screen.

## Hardware Requirements

- [Adafruit Feather HUZZAH ESP8266](https://www.adafruit.com/product/2821)
- 4.2" e-paper display module (GDEY042T81)
- Micro USB cable (for programming and power)
- Jumper wires for connections

## Wiring Diagram

Connect the e-paper display to the Feather HUZZAH as follows:

| E-Paper Pin | ESP8266 Feather HUZZAH Pin |
|-------------|----------------------------|
| BUSY        | GPIO #16                   |
| RESET       | GPIO #5                    |
| DC          | GPIO #4                    |
| CS          | GPIO #15                   |
| SCL         | GPIO #14 (SCK)             |
| SDA         | GPIO #13 (MOSI)            |
| GND         | GND                        |
| VCC         | 3V                         |

## Software Requirements

### Libraries

Install the following libraries using the Arduino Library Manager:

- ESP8266WiFi
- ESP8266HTTPClient
- ArduinoJson (version 6.x)
- GxEPD2 (for the e-paper display)
- Adafruit GFX

### API Key

You'll need an API key from [OpenWeatherMap](https://openweathermap.org/api). Sign up for a free account to get your API key.

## Setup Instructions

1. Clone or download this repository.

2. Create a `Credentials.h` file in the same directory as the main sketch with the following content:
   ```cpp
   #ifndef CREDENTIALS_H
   #define CREDENTIALS_H
   
   // WiFi credentials
   const char* ssid = "YourWiFiSSID";
   const char* password = "YourWiFiPassword";
   
   // OpenWeatherMap API key
   const char* apiKey = "YourOpenWeatherMapAPIKey";
   
   #endif // CREDENTIALS_H
   ```

3. Replace `YourWiFiSSID`, `YourWiFiPassword`, and `YourOpenWeatherMapAPIKey` with your actual WiFi credentials and API key.

4. Open the `ESP8266-Huzzah-Feather.ino` file in the Arduino IDE.

5. Modify the `city` variable to show weather for your desired location:
   ```cpp
   const char* city = "YourCity";
   ```

6. Connect your Feather HUZZAH to your computer using a micro USB cable.

7. Select the appropriate board and port in the Arduino IDE:
   - Board: "Adafruit Feather HUZZAH ESP8266"
   - Port: The COM port where your board is connected

8. Upload the sketch to your board.

## How It Works

1. The device connects to your WiFi network using the provided credentials.
2. It makes a secure HTTPS request to the OpenWeatherMap API to fetch current weather data.
3. The JSON response is parsed to extract the city name, temperature, and weather description.
4. This information is displayed on the e-paper screen:
   - City name (large font)
   - Weather description (medium font)
   - Temperature in Celsius (small font)
5. The e-paper display will retain the information even when power is disconnected.

## Customization

- Change the display rotation by modifying the `display.setRotation()` value.
- Adjust the fonts by changing the references to different font files.
- Modify the layout by adjusting the text positioning calculations.

## Troubleshooting

- Check the Serial Monitor (115200 baud) for debugging information.
- If you see "Unable to connect to HTTPS server", verify your internet connection.
- If you see "JSON parsing failed", check your API key and city name.

## Power Considerations

The e-paper display only requires power when updating. This makes this project ideal for battery-powered applications. For extended battery life, consider:

1. Adding deep sleep between updates
2. Updating the display less frequently
3. Using a LiPo battery with the built-in charging circuit of the Feather HUZZAH

## License

This project is released under the MIT License.