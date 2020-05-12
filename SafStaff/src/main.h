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

void updateGyro();

void detectMenu();

void timeMode();
void rainbowMode();
void twinkleMode();
void centre_out_dot(uint8_t y, CRGB color, uint32_t wait, bool first) ;

#define DATA_PIN    13
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    192
#define NUM_LED_SETS 48
#define NUM_LED_IN_SET 4 ///must be leds/sets
#define BRIGHTNESS  100
#define FRAMES_PER_SECOND 120

MPU6050 mpu6050(Wire);  // create gyro object
SSD1306Wire display(0x3c, 5, 4);
CRGB rawleds[NUM_LEDS];
CRGBSet leds(rawleds, NUM_LEDS);
struct CRGB * ledarray[NUM_LED_SETS];
uint8_t sizearray[NUM_LED_SETS];
unsigned long timer;

Gyro gy;


//detect menu parameters
bool MENU_MODE = false;
int lastZ = 0;
bool posRotation = false;
int iterCount = 0;
int switchCount = 0;
int requiredSwitches=12;
int detectMenuTimeout = 40;
float angTol = 10;

//menu parameters
int MENU_OPTION = 1;
int menuCount = 0;
uint8_t bCount = 0;
int bCountMax = 8;
int confirmCount = 0;
int confirmOption = 1;


void resetVariables() {
    leds.fill_solid(CHSV(0,0,0));
}



