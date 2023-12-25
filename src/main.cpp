/////////////////////////////
//////////// TODO ///////////
/////////////////////////////
// - Add OLED display
// - Add MQTT
// - Add OTA
// - Add Webserver
// - Add Websocket
// - Add weather
// - Add BLE?
// - Add Custom image
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
//#include "images.h"
/////////////////////////////
///////// CONFIGS ///////////
//#define led_pin 2        // Uncomment this line and change the number to use a different pin for the LED
#define button D0           // Digital pin for the button
#define sensor 6           // Analog pin for the light sensor
#define oled_sda 4         // SDA pin for the OLED display
#define oled_scl 5         // SCL pin for the OLED display
//#define flipped          // Uncomment this line to flip the OLED display
/////////////////////////////
///////// DEFINES ///////////
/////////////////////////////
#ifdef led_pin
  #define led led_pin
#else
  #define led LED_BUILTIN
#endif
SSD1306Wire display(0x3c, oled_sda, oled_scl);
bool new_message = false;
bool night_mode = false;
bool printed = false;
bool standby_msg = false;
int led_state = 0;
int light_threshold = 100;
unsigned long led_timer = 0;
unsigned long oled_timer = 0;
unsigned long weather_timer = 0;
unsigned long msg_timer = 0;
/////////////////////////////
//////// FUNCTIONS //////////
/////////////////////////////
void heartbeat() {                                              // Heartbeat
  for(int dutyCycle = 0; dutyCycle < 255; dutyCycle++){         // Increase brightness
    analogWrite(led, dutyCycle);
    delay(10);
  }
  for(int dutyCycle = 255; dutyCycle > 0; dutyCycle--){         // Decrease brightness
    analogWrite(led, dutyCycle);
    delay(10);
  }
}

void standby() {                                                // Standby mode
  if(weather_timer + 1800 < (millis() / 1000)) {                // Check for weather every half hour
    weather_timer = millis() / 1000;
    //get weather
  }
  if(oled_timer + 10 < (millis() / 1000) && not standby_msg) {
    oled_timer = millis() / 1000;
    standby_msg = true;
    display.clear();
    display.drawString(0, 0, "Time and Weather");
  }
  if(oled_timer + 10 < (millis() / 1000) && standby_msg) {
    oled_timer = millis() / 1000;
    standby_msg = false;
    display.clear();
    display.drawString(0, 0, "Quote.");
  }
}

void nightmode() {                                             // Turn everything off and go to sleep
  display.clear();
  analogWrite(led, 0);
  while(analogRead(sensor) < light_threshold) {                 // Wait until the light sensor is above the threshold
    delay(60000);
  }
  night_mode = false;
  }

void newmessage() {                                             // Display the new message
  display.clear();
  display.drawString(0, 0, "New message!");
  if(led_timer + 1 < (millis() / 1000) && led_state == LOW) {
    led_state = HIGH;
    led_timer = millis() / 1000;
    digitalWrite(led, HIGH);
  }
  if(led_timer + 1 < (millis() / 1000) && led_state == HIGH) {
    led_state = LOW;
    led_timer = millis() / 1000;
    digitalWrite(led, LOW);
  }
}

void running() {                                                 // Check for new messages
  if(msg_timer + 15 < (millis() / 60000)) {                      // Check for messages every 15 seconds
    msg_timer = millis() / 1000;
    //check for messages
  }
  new_message = true;
  if(not new_message ) {
    heartbeat();
    standby();
  }
  if(new_message) {
    newmessage();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  display.init();
  #ifdef flipped
    display.flipScreenVertically();
  #endif
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setColor(WHITE);
  WiFiManager wifiManager;
  wifiManager.autoConnect("MomsMantra");
}

void loop() {
  /*
  if(analogRead(sensor) < light_threshold) {                    // If the light sensor is below the threshold, turn on the LED
    night_mode = true;                                          // Set night mode to true
  }
  else {                                                        // If the light sensor is above the threshold, turn off the LED
    night_mode = false;                                         // Set night mode to false
  }
  */
  if(digitalRead(button) == 0) {                                // If the button is pressed, set unread to false
    new_message = false;
  }
  if(not night_mode) {                                          // If there is no new message, standby mode
    running();
  }
  if(night_mode) {                                              // If night mode detected, run night mode function
    nightmode();
  }
  display.display();                                            // Display the OLED display
}