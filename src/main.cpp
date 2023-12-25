/////////////////////////////
//////////// TODO ///////////
/////////////////////////////
// - Add OLED display
// - Add MQTT
// - Add OTA
// - Add WiFi
// - Add Webserver
// - Add Websocket
// - Add BLE?
// - Add Custom image
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
#define sensor 0           // Analog pin for the light sensor
#define oled_sda 4         // SDA pin for the OLED display
#define oled_scl 5         // SCL pin for the OLED display
/////////////////////////////
///////// DEFINES ///////////
/////////////////////////////
#ifdef led_pin
  #define led led_pin
#else
  #define led LED_BUILTIN
#endif
bool new_message = false;
bool night_mode = false;
int light_threshold = 100;
SSD1306Wire display(0x3c, oled_sda, oled_scl);

/////////////////////////////
//////// FUNCTIONS //////////
/////////////////////////////
void setup() {
  pinMode(led, OUTPUT);

}

void loop() {
  if (analogRead(sensor) < light_threshold) {                   // If the light sensor is below the threshold, turn on the LED
    night_mode = true;                                          // Set night mode to true
  }
  else {                                                        // If the light sensor is above the threshold, turn off the LED
    night_mode = false;                                         // Set night mode to false
  }
  if(not new_message) {                                         // If there is no new message, standby
    if( not night_mode ) {                                      // If it is not night mode, heartbeat
      for(int dutyCycle = 0; dutyCycle < 255; dutyCycle++){     // Increase brightness
        analogWrite(led, dutyCycle);
        delay(10);
      }
      for(int dutyCycle = 255; dutyCycle > 0; dutyCycle--){     // Decrease brightness
        analogWrite(led, dutyCycle);
        delay(10);
      }
    }
    else {                                                      // If it is night mode, no heartbeat and turn off the OLED display
      display.clear();
    }

  }
  display.display();
}