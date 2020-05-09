#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <SSD1306Wire.h>
#include <FastLED.h>
#include <string>
#include <sstream>
#include <iostream>
#include <list>

using namespace std;

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
#define BRIGHTNESS  5
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


float lastGyroX = 0;
float lastGyroY = 0;
float lastGyroZ = 0;

void resetVariables() {
    //lastGyroZ = 0;
    lastGyroX = mpu6050.getGyroAngleX();
    lastGyroY = mpu6050.getGyroAngleY();
    lastGyroZ = mpu6050.getGyroAngleZ();
    leds.fill_solid(CHSV(0,0,0));
}



struct gyro_t_pt
{
    float angleX;
    float angleY;
    float angleZ;
    float accX;
    float accY;
    float accZ;
};

list<gyro_t_pt> gyroHistory;

struct gyro_calcs
{
    //avg values over history buffer
    float averageXAcc;
    float averageYAcc;
    float averageZAcc;

    float currentXSpeed;
    float currentYSpeed;
    float currentZSpeed;
    float currentXYZSpeed;
    
    float maxZAcc = -100;
    float minZAcc = 100;
    float rangeZacc;
};

gyro_calcs updateGyroCals()
{
    gyro_calcs currentCalc;
    //return empty object if history is too short
    if (gyroHistory.size()<9){return currentCalc;}

    float avgXAcc = 0;
    float avgYAcc = 0;
    float avgZAcc = 0;

    float minXAngle = 1e16;
    float minYAngle = 1e16;
    float minZAngle = 1e16;
    float maxXAngle = -1e16;
    float maxYAngle = -1e16;
    float maxZAngle = -1e16;

    int loopNum = 0;
    int onlyFirstNum = 5;
    std::list<gyro_t_pt>::iterator it;
    for (it = gyroHistory.begin(); it != gyroHistory.end(); it++)
    {
        //if we onlt care for the n most recent values
        if (loopNum<onlyFirstNum) 
        {
            if (minXAngle > it->angleX){minXAngle = it->angleX;}
            if (maxXAngle < it->angleX){maxXAngle = it->angleX;}            
            if (minYAngle > it->angleY){minYAngle = it->angleY;}
            if (maxYAngle < it->angleY){maxYAngle = it->angleY;}            
            if (minZAngle > it->angleZ){minZAngle = it->angleZ;}
            if (maxZAngle < it->angleZ){maxZAngle = it->angleZ;}
            
            if (currentCalc.maxZAcc < it->accZ){currentCalc.maxZAcc = it->accZ;}
            if (currentCalc.minZAcc > it->accZ){currentCalc.minZAcc = it->accZ;}
        }

        avgXAcc += it->accX;
        avgYAcc += it->accY;
        avgZAcc += it->accZ;
             
        //add lots of other calcs!!!!
    }
    currentCalc.averageXAcc = avgXAcc/(gyroHistory.size());
    currentCalc.averageYAcc = avgYAcc/(gyroHistory.size());
    currentCalc.averageZAcc = avgZAcc/(gyroHistory.size());
    currentCalc.rangeZacc = currentCalc.maxZAcc - currentCalc.minZAcc;

    currentCalc.currentXSpeed = maxXAngle - minXAngle;
    currentCalc.currentYSpeed = maxYAngle - minYAngle;
    currentCalc.currentZSpeed = maxZAngle - minZAngle;
 
    currentCalc.currentXYZSpeed = currentCalc.currentXSpeed + currentCalc.currentYSpeed + currentCalc.currentZSpeed;

    return currentCalc;
}

gyro_calcs updateGyro() {
    mpu6050.update();
    gyro_t_pt currentGyro{mpu6050.getGyroAngleX(), mpu6050.getGyroAngleY(),mpu6050.getGyroAngleZ(),mpu6050.getAccX(),mpu6050.getAccY(),mpu6050.getAccZ()};
    gyroHistory.push_front(currentGyro);
    if (gyroHistory.size()>10){gyroHistory.pop_back();}
    return updateGyroCals();
}