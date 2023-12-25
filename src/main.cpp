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
#include "OpenWeatherMapCurrent.h"
#include <JsonListener.h>
#include <time.h>
#include <ESP8266mDNS.h>
#include "PubSubClient.h"
//#include "images.h"
/////////////////////////////
///////// CONFIGS ///////////
//#define led_pin 2        // Uncomment this line and change the number to use a different pin for the LED
#define button D3          // Digital pin for the button
#define sensor A0          // Analog pin for the light sensor
#define oled_sda 12        // SDA pin for the OLED display
#define oled_scl 14        // SCL pin for the OLED display
//#define flipped          // Uncomment this line to flip the OLED display
//#define OTA              // Uncomment this line to enable OTA
#define TZ 2               // Define timezone
#define DST_MN 60          // Define daylight savings time
String KEY = "2edb13fe8193c967c9efba28f56b2f0f";        // OpenWeatherMap API key
String MAP = "4671524";    // OpenWeatherMap location ID
String LANG = "en";        // OpenWeatherMap language
boolean IS_METRIC = false; // Imperial: false, Metric: true
const char* mqtt_server = "test.mosquitto.org";
/////////////////////////////
///////// DEFINES ///////////
/////////////////////////////
#ifdef led_pin
  #define led led_pin
#else
  #define led LED_BUILTIN
#endif
SSD1306Wire display(0x3c, oled_sda, oled_scl);
OpenWeatherMapCurrent wclient;
OpenWeatherMapCurrentData data;
WiFiClient espClient;
PubSubClient client(espClient);
bool new_message = false;
bool night_mode = false;
bool printed = false;
bool standby_msg = false;
int led_state = 0;
int light_threshold = 100;
long lastMsg = 0;
char msg[50];
int value = 0;
unsigned long led_timer = 0;
unsigned long oled_timer = 0;
unsigned long weather_timer = 0;
unsigned long msg_timer = 0;
unsigned long time_sleep = 0;
/////////////////////////////
//////// FUNCTIONS //////////
/////////////////////////////
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  display.clear();
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    //display.clear();
    msg += (char)payload[i];
  }
  // display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawStringMaxWidth(0,0,120,msg); //ESP8266 w/OLED
  display.display();
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    uint32_t chipid=ESP.getChipId();
    char clientid[25];
    snprintf(clientid,25,"WIFI-Display-%08X",chipid); //this adds the mac address to the client for a unique id
    Serial.print("Client ID: ");
    Serial.println(clientid);
    if (client.connect(clientid)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("Say", "-t 'hello world'");
      // ... and resubscribe
      client.subscribe("samwarr3979");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void heartbeat() {                                              // Heartbeat
  for(int dutyCycle = 0; dutyCycle < 255; dutyCycle++){         // Increase brightness
    analogWrite(led, dutyCycle);
    delay(10);
  }
  yield();
  for(int dutyCycle = 255; dutyCycle > 0; dutyCycle--){         // Decrease brightness
    analogWrite(led, dutyCycle);
    delay(10);
  }
}

void standby() {                                                // Standby mode
  #ifdef OTA
    ArduinoOTA.handle();
  #endif
  if(weather_timer + 1800 < (millis() / 1000)) {                // Check for weather every half hour
    weather_timer = millis() / 1000;
    wclient.updateCurrentById(&data, KEY, MAP); 
  }
  if(oled_timer + 10 < (millis() / 1000) && not standby_msg) {
    oled_timer = millis() / 1000;
    standby_msg = true;
    display.clear();
    Serial.printf("main: %s\n", data.main.c_str());
    Serial.printf("description: %s\n", data.description.c_str());
    Serial.printf("icon: %s\n", data.icon.c_str());
    Serial.printf("temp: %f\n", data.temp);
    Serial.printf("humidity: %d\n", data.humidity);
    Serial.printf("tempMin: %f\n", data.tempMin);
    Serial.printf("tempMax: %f\n", data.tempMax);
    time_t time = data.observationTime;
    Serial.printf("observationTime: %d, full date: %s", data.observationTime, ctime(&time));
  }
  if(oled_timer + 10 < (millis() / 1000) && standby_msg) {
    oled_timer = millis() / 1000;
    standby_msg = false;
    display.clear();
    //display last message
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
    delay(250);
  }
}

void newmessage() {                                             // Display the new message
  display.clear();
  display.drawString(0, 0, "New message!");
  if(led_timer + 1000 < (millis()) && led_state == LOW) {
    led_state = HIGH;
    led_timer = millis();
    digitalWrite(led, HIGH);
    Serial.println("LED on");
  }
  yield();
  if(led_timer + 1000 < (millis()) && led_state == HIGH) {
    led_state = LOW;
    led_timer = millis();
    digitalWrite(led, LOW);
    Serial.println("LED off");
  }
}

void running() {                                                 // Check for new messages
  yield();
  if(msg_timer + 15 < (millis() / 60000)) {                      // Check for messages every 15 min
    msg_timer = millis() / 1000;
    //check for messages
    new_message = true;
  }
  if(not new_message ) {
    heartbeat();
    standby();
  }
  if(new_message) {
    newmessage();
  }
}

void setup() {
  yield();
  Serial.begin(115200);
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
  wifiManager.autoConnect("MomsMantra");
  #ifdef OTA
    ArduinoOTA.setPort(2580);
    ArduinoOTA.setHostname("MomsMantra");
    ArduinoOTA.setPassword("samwise");
    ArduinoOTA.begin();
  #endif
  wclient.setLanguage(LANG);
  wclient.setMetric(IS_METRIC);
  wclient.updateCurrentById(&data, KEY, MAP);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();
}

void loop() {
  client.loop();
  yield();
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