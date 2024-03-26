# ESP8266 OLED Weather Display Message Board
## Description
This is an Arduino project that uses an ESP8266 board to display weather information and receive MQTT messages. The project uses your OpenWeatherMap API to fetch current weather data and MQTT to retrieve messages and act as a message board. The project then switches between displaying the current weather information and the last recieved message on an 128x64 OLED display connected to the ESP8266 board. Running `python3 publisher.py` in the `src` folder starts a prompter that when given a message, publishes it to the MQTT server. The ESP8266 board then recieves the message and displays it on the OLED display. The ESP8266 board also has a button that when pressed, acknowledges the message and publishes a confirmation message to the MQTT server. The ESP8266 board also has a notification LED that fades in and out when a new message is recieved. The ESP8266 board also has a light sensor that when it detects that it is dark, turns off the display and notification LED. The ESP8266 board also has WiFiManager and ArduinoOTA enabled to allow for easy WiFi setup and OTA updates.

## Features
- Fading notification LED upon new message
- Weather updates every 10 minutes
- MQTT message delivery is instant
- Optional light sensor to automatically turn off the display and notifications at night
- Button acknowledgement of new message published a confirmation message to the MQTT server
- Pressing the button while no new messages are recieved will publish a static custom message to the MQTT server
- WiFiManager allows for easy WiFi setup without having to hardcode SSID and password and also allows for easy WiFi reconfiguration and setup without having to reprogram the board or needing to connect to the board via serial
- ArduinoOTA allows for easy OTA updates without having to connect to the board via serial

## Configurations
- `button` - The pin number of the button
- `led_pin` - The pin number of the notification LED
- `sensor` - The pin number of the light sensor
- `oled_sda` and `oled_scl` - The SDA and SCL pins of the OLED display
- `flipped` - Uncomment this line to flip your display if it is upside down
- `OTA` - Uncomment this line to enable OTA updates
- `SECRET_SERVER` - The address of the MQTT server, defined in secretfile.h

## Prerequisites
You will need the following libraries installed on your Arduino IDE or PlatformIO:
- ESP8266 and ESP32 OLED driver for SSD1306 displays
- JsonStreamingParser
- PubSubClient
- WiFiManager
- ESP8266 Weather Station
You will also need to create a secretfile.h and secretfile.py files that contains your OpenWeatherMap API key and MQTT server details along with other required passwords and stuff. I have included the template files for this (secretfile_example.h and secretfile.py) in their respective directories.

## Installation
- Clone the repository to your local machine.
- Open the project in your IDE.
- Install the necessary libraries.
- Create required secrets files and add necessary info.
- Upload the code to your ESP8266 board.

## License
This project is licensed under the GNU General Public License v3.0 - see the LICENSE.md file for details.

## TODO
- Add custom weather icons for different weather conditions
- Local network web server for displaying weather information and messages