/////////////////////////////
//////////// TODO ///////////
/////////////////////////////
// - Add OLED display
// - Add MQTT
// - Add OTA
// - Add WiFi
// - Add Webserver
// - Add Websocket
// - Add weather
// - Add BLE?
// - Add Custom image
// - Add Deep sleep
/////////////////////////////
///////// INCLUDES //////////
/////////////////////////////
#include <Arduino.h>
#include <Wire.h>
#include <SSD1306Wire.h>
//#include "images.h"
/////////////////////////////
///////// CONFIGS ///////////
//#define led_pin 2        // Uncomment this line and change the number to use a different pin for the LED
#define button 1           // Digital pin for the button
#define sensor 0           // Analog pin for the light sensor
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
int light_threshold = 100;
unsigned long timer_on = 0;
unsigned long timer_off = 0;
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
  if(not printed) {
    display.clear();
    display.drawString(0, 0, "Standby");
    printed = true;
  }
}

void nightmode() {                                             // Turn everything off and go to sleep
  display.clear();
  analogWrite(led, 0);
  delay(10000);
  // add deep sleep functionality?
}

void newmessage() {                                             // Display the new message
  display.clear();
  display.drawString(0, 0, "New message!");
  if(timer_on + 1000 < millis()) {
    digitalWrite(led, HIGH);
    timer_off = millis();
  }
  if(timer_off + 1000 < millis()) {
    digitalWrite(led, LOW);
    timer_on = millis();
  }

}

void running() {                                               // Check for new messages
  //check for messages at intervals
  new_message = true;
  if( not new_message ) {
    heartbeat();
    standby();
  }
  else {
    newmessage();
  }
}

void setup() {
  pinMode(led, OUTPUT);
  display.init();
  #ifdef flipped
    display.flipScreenVertically();
  #endif
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setColor(WHITE);
}

void loop() {
  if(analogRead(sensor) < light_threshold) {                    // If the light sensor is below the threshold, turn on the LED
    night_mode = true;                                          // Set night mode to true
  }
  else {                                                        // If the light sensor is above the threshold, turn off the LED
    night_mode = false;                                         // Set night mode to false
  }
  //if(digitalRead(button) == 0) {                                // If the button is pressed, set unread to false
  //  new_message = false;
  //}
  if(not night_mode) {                                          // If there is no new message, standby mode
    running();
  }
  else {                                                        // If there is a new message, display it
    //nightmode();
  }
  display.display();                                            // Display the OLED display
}