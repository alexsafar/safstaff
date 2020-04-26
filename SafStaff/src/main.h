#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <SSD1306Wire.h>
#include <FastLED.h>
#include <string>
#include <sstream>
#include <iostream>

void initialiseLedSet();

void displayGyro();
void displayMenu();
void displayLED();

void detectMenu();
void detectOff();

void timeMode();
void rainbowMode();
void twinkleMode();
void blockMode();
void centre_out_dot(uint8_t y, CRGB color, uint32_t wait, bool first) ;

#define DATA_PIN    13
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    192
#define NUM_LED_SETS 48
#define NUM_LED_IN_SET 4 ///must be leds/sets
#define BRIGHTNESS  50
#define FRAMES_PER_SECOND 120

MPU6050 mpu6050(Wire);  // create gyro object
SSD1306Wire display(0x3c, 5, 4);
CRGB rawleds[NUM_LEDS];
CRGBSet leds(rawleds, NUM_LEDS);
struct CRGB * ledarray[NUM_LED_SETS];
uint8_t sizearray[NUM_LED_SETS];
unsigned long timer;

//offmode params
float zInOffModePrevious = 0;

//time mode params
int tMode_gyroCount1 = 0;
int tMode_gyroCount2 = 0;
int tMode_gyroMiss1 = 0;
int tMode_gyroMiss2 = 0;
int tMode_gyroActivateCount = 120;

//rainbow params
uint8_t hue = 0;
uint8_t delta = 10;

//twinklemode params
int brightArr [NUM_LEDS] = {};
int onOffArr [NUM_LEDS] = {};
int colourArr [NUM_LEDS] = {};
float lastAccZtwink = 0;

//blockmode params
float lastGyroZ = 0;
uint8_t bMode_cycleIter = 0; 
int bMode_gyroCount = 0;
int bMode_gyroActivateCount = 20;

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
uint8_t bBrightness = 255;
uint8_t bCount = 0;
int bCountMax = 8;
int confirmCount = 0;
int confirmOption = 1;
uint8_t confirmBrightness = 255;

void resetVariables() {
    lastGyroZ = 0;
}



