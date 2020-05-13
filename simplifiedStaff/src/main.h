#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <SSD1306Wire.h>
#include <FastLED.h>


void initialiseLedSet();
void displayGyro();
void rainbowMode();
void solidMode();

#define DATA_PIN    13
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    192
#define BRIGHTNESS  100

MPU6050 mpu6050(Wire);  // create gyro object
SSD1306Wire display(0x3c, 5, 4);
CRGB rawleds[NUM_LEDS];
CRGBSet leds(rawleds, NUM_LEDS);




