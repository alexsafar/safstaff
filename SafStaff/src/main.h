#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <SSD1306Wire.h>
#include <FastLED.h>
#include <string>
#include <sstream>
#include <iostream>
#include <list>
#include "gyr.h"

void initialiseLedSet();
void displayGyro();
void displayMenu();
void detectMenu();
void updateGyro();
void timeMode();
void rainbowMode();
void twinkleMode();
void centre_out_dot(uint8_t y, CRGB color, uint32_t wait, bool first) ;

#define DATA_PIN    13
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    192
#define NUM_LED_SETS 48
#define NUM_LED_IN_SET 4 ///must be NUM_LEDS/NUM_LED_SETS
#define BRIGHTNESS  100
#define FRAMES_PER_SECOND 120

MPU6050 mpu6050(Wire);  // create gyro object
SSD1306Wire display(0x3c, 5, 4);
CRGB rawleds[NUM_LEDS];
CRGBSet leds(rawleds, NUM_LEDS);
struct CRGB * ledarray[NUM_LED_SETS];
uint8_t sizearray[NUM_LED_SETS];

Gyro gy;
unsigned long timer;
bool MENU_MODE;
int MENU_OPTION;

void resetVariables() {
    leds.fill_solid(CHSV(0,0,0));
}



