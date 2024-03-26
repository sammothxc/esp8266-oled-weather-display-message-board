/*
    ESP8266 OLED Weather Display Message Board
    Copyright (C) 2023  Sam Warr

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/////////////////////////////
//////////// TODO ///////////
/////////////////////////////
// - Add weather icons
// - Fix MQTT publish message
/////////////////////////////
///////// INCLUDES //////////
/////////////////////////////
#include <Arduino.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "OpenWeatherMapCurrent.h"
#include <JsonListener.h>
#include <time.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include "secretfile.h"
//#include "images.h"
/////////////////////////////
///////// CONFIGS ///////////
#define led_pin 5          // Uncomment this line and change the number to use a different pin for LED
#define button 6           // Digital pin for the button
#define sensor A0          // Analog pin for the light sensor
#define oled_sda 2         // SDA pin for the OLED display
#define oled_scl 14        // SCL pin for the OLED display
#define flipped          // Uncomment this line to flip the OLED display
#define OTA              // Uncomment this line to enable OTA
#define TZ 2               // Define timezone
#define DST_MN 60          // Define daylight savings time
String LANG = "en";        // OpenWeatherMap language
boolean IS_METRIC = false; // Imperial: false, Metric: true
//#define light_sensor      // Uncomment this line to enable the light sensor
const char* mqtt_server = SECRET_SERVER;
/////////////////////////////
///////// DEFINES ///////////
/////////////////////////////
#ifdef led_pin
  #define led led_pin
#else
  #define led LED_BUILTIN
#endif
SSD1306Wire display(0x3c, oled_sda, oled_scl, GEOMETRY_128_64);
OpenWeatherMapCurrent wclient;
OpenWeatherMapCurrentData data;
WiFiClient espClient;
PubSubClient client(espClient);
String msg = " :)";
bool new_message = false;
bool night_mode = false;
bool printed = false;
bool standby_msg = false;
bool led_state = false;
int light_threshold = 100;
int value = 0;
unsigned long led_timer = 0;
unsigned long oled_timer = 0;
unsigned long weather_timer = 0;
unsigned long msg_timer = 0;
unsigned long time_sleep = 0;
/////////////////////////////
//////// FUNCTIONS //////////
/////////////////////////////
void newmessage() {                                             // Display the new message
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawStringMaxWidth(0,0,128,"New Message!");
  display.display();
  yield();
}

void display_msg() {                                            // Display the message
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawStringMaxWidth(0,0,120,msg);
  display.display();
  yield();
}

void callback(char* topic, byte* payload, unsigned int length) {
  msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  new_message = true;
}

void reconnect() {
  while (!client.connected()) {
    char clientid[25];
    snprintf(clientid, 25, SECRET_HOSTNAME);
    if (client.connect(clientid)) {
      client.subscribe(SECRET_TOPIC);
    } else {
      delay(5000);
    }
  }
}

void notify() {                                                   // notify
  newmessage();
  for(int dutyCycle = 0; dutyCycle < 255; dutyCycle++){           // Increase brightness
    analogWrite(led, dutyCycle);
    if(digitalRead(button) == LOW) {                              // If the button is pressed, set unread to false
      new_message = false;
      display_msg();
      client.publish("Say", "-t 'message read'");
      delay(4000);
    }
    delay(10);
  }
  yield();
  for(int dutyCycle = 255; dutyCycle > 0; dutyCycle--){            // Decrease brightness
    analogWrite(led, dutyCycle);
    if(digitalRead(button) == LOW) {                               // If the button is pressed, set unread to false
      new_message = false;
      display_msg();
      client.publish("Say", "-t 'message read'");
      delay(4000);
    }
    delay(10);
  }
}

void standby() {                                                // Standby mode
  #ifdef OTA
    ArduinoOTA.handle();
  #endif
  if(weather_timer + 1800 < (millis() / 1000)) {                // Check for weather every half hour
    weather_timer = millis() / 1000;
    wclient.updateCurrentById(&data, SECRET_API_KEY, SECRET_LOCATION_ID); 
  }
  if(oled_timer + 5 < (millis() / 1000) && not standby_msg) {
    oled_timer = millis() / 1000;
    standby_msg = true;
    display.clear();
    //Serial.printf("icon: %s\n", data.icon.c_str());
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Now: ");
    display.drawString(27, 0, data.main.c_str());
    display.drawString(0, 16, "1 Hr: ");
    display.drawString(27, 16, data.description.c_str());
    display.drawString(0, 32, "Temp/Hum: ");
    display.drawString(56, 32, String(int(round(data.temp))).c_str());
    display.drawString(72, 32, String(int(round(data.humidity))).c_str());
    display.drawString(0, 48, "Hi/Lo: ");
    display.drawString(30, 48, String(int(round(data.tempMax))).c_str());
    display.drawString(47, 48, String(int(round(data.tempMin))).c_str());
  }
  if(oled_timer + 5 < (millis() / 1000) && standby_msg) {
    oled_timer = millis() / 1000;
    standby_msg = false;
    display_msg();
  }
}

void nightmode() {                                             // Turn everything off and go to sleep
  display.clear();
  analogWrite(led, 0);
  time_sleep = millis();
  while(not night_mode) {                                      // Wait until light sensor is above the threshold
    if(analogRead(sensor) < light_threshold) {
      night_mode = false;
    }
    yield();
    delay(300);
  }
}

void running() {                                                 // Check for new messages
  yield();
  if(not new_message ) {
    standby();
    if(digitalRead(button) == LOW) {                             // If the button is pressed, set unread to false
      client.publish("Say", "-t 'sam says hi!'");
      display.clear();
      display.setFont(ArialMT_Plain_16);
      display.drawStringMaxWidth(0,0,128,"Message Sent!");
      display.display();
      delay(1000);
    }
  }
  if(new_message) {
    newmessage();
  }
}

void setup() {
  yield();
  pinMode(led, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(sensor, INPUT);
  display.init();
  #ifdef flipped
    display.flipScreenVertically();
  #endif
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setColor(WHITE);
  WiFiManager wifiManager;
  wifiManager.autoConnect(SECRET_HOSTNAME);
  #ifdef OTA
    ArduinoOTA.setPort(2580);
    ArduinoOTA.setHostname(SECRET_HOSTNAME);
    ArduinoOTA.setPassword(SECRET_PASSWORD);
    ArduinoOTA.begin();
  #endif
  wclient.setLanguage(LANG);
  wclient.setMetric(IS_METRIC);
  wclient.updateCurrentById(&data, SECRET_API_KEY, SECRET_LOCATION_ID);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();
  delay(1000);
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawStringMaxWidth(0,0,128,WiFi.localIP().toString());
  display.display();
  yield();
  msg = SECRET_HOSTNAME + msg;
  delay(3000);
}

void loop() {
  client.loop();
  yield();
  #ifdef light_sensor
    if(analogRead(sensor) < light_threshold) {                    // If the light sensor is below the threshold, turn on the LED
      night_mode = true;                                          // Set night mode to true
    }
    else {                                                        // If the light sensor is above the threshold, turn off the LED
      night_mode = false;                                         // Set night mode to false
    }
  #endif
  if(new_message) {                                             // If there is a new message, run new message function
    notify();
  }
  if(not night_mode) {                                          // If there is no new message, standby mode
    running();
  }
  if(night_mode) {                                              // If night mode detected, run night mode function
    nightmode();
  }
  display.display();                                            // Display the OLED display
}