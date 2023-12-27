# Project Title
## Description
This is an Arduino project that uses an ESP8266 board to display weather information and receive MQTT messages. The project uses your OpenWeatherMap API to fetch current weather data and MQTT to retrieve messages and act as a message board. The project then switches between displaying the current weather information and the last recieved message on an 128x64 OLED display connected to the ESP8266 board.

## Features
- Fading notification LED upon new message
- Weather updates every 10 minutes
- MQTT message delivery is instant
- Optional light sensor to automatically turn off the display and notifications at night
- Button acknowledgement of new message published a confirmation message to the MQTT server
- Pressing the button while no new messages are recieved will publish a static custom message to the MQTT server
- WiFiManager allows for easy WiFi setup without having to hardcode SSID and password and also allows for easy WiFi reconfiguration and setup without having to reprogram the board or needing to connect to the board via serial
- ArduinoOTA allows for easy OTA updates without having to connect to the board via serial

## Prerequisites
You will need the following libraries installed on your Arduino IDE or PlatformIO:
- ESP8266 and ESP32 OLED driver for SSD1306 displays
- JsonStreamingParser
- PubSubClient
- WiFiManager
- ESP8266 Weather Station
You will also need to create a secrets.h file that contains your OpenWeatherMap API key and MQTT server details along with other required passwords and stuff.

## Installing
- Clone the repository to your local machine.
- Open the project in your IDE.
- Install the necessary libraries.
- Create a secrets.h file and add your OpenWeatherMap API key and MQTT server details.
- Upload the code to your ESP8266 board.

## License
This project is licensed under the GNU General Public License v3.0 - see the LICENSE.md file for details.

## TODO
- Add custom weather icons for different weather conditions
- Local network web server for displaying weather information and messages
