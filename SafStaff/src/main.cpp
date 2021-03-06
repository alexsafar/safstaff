#include "main.h"

////////////////////////
// This code is for an LED contact staff 
// 
// It requires the hardware (in each hub):
// Microcontroller (chip that's in control) = ESP32 (wemos lolin32 oled)
// 3.2m of LEDs (individually addressible LED) = WS2812b (60 per m)
// Accelerometer (detects angle and acceleration) = MPU6050
// 2x 18650 batteries
// 2x battery holders
// usb charging station
//
// The project is made in the free software Visual Studio Code 1.35.1 on Linux (tested on Windows too)
// PlatformIO extension must be installed (through VSCode's 'Extensions' functionality)
// And the following libraries must be installed
// - Arduino (may come with PlatformIO?)
// - Wire (may come with PlatformIO?)
// - MPU6050_tockn
// - SSD1306Wire
// - FastLED
//
// There are 5 essential files in this project:
// main.h and main.cpp - these have the main code for running everything
// gyr.h and gyr.cpp - these contain custom ways of storing and processing gyro data
// platformio.ini - contains some configuration data
//
// This code has the general structure:
// setup() initialises everything (when first turned on)
// loop() is called constantly and is the main corridoor in the architecture
// detectMenu() checks to see if we want to activate menu mode 
// displayMenu() changes which light mode, depending on orientation (upright, horizontal, somewhere inbetween)
//
// rainbowMode() shows a simple cycle of colours which display over the rangle of LEDs
// timeMode() shows a block colour accross all LEDs which changes with time
//        - it also has a special mode when the staff rolls for a long time, calling:
//        the centre_out_dot function which shows lights expanding from the centre of the unit outwards, then in again
// twinkleMode() shows random LEDs activate with random colours, then fade
//        - it also has a special mode activated when the staff suddenly moves along it's axis, or stopps spinning,
//        where it then fills entirely with a block colour, before fading and going back to twinkle mode
//
// updateGyro() calls functions in gyr which store the last 10 accelerometer values
//        - these are evaluated to produce speed and change in acceleration
//
////////////////////////



////////////////////////
// Setup is called only once - when the chip first turns on
////////////////////////
void setup() {
  Serial.begin(921600); // The baud rate is set to 921600, which needs to be also set in platformio.ini (monitor_speed = 921600) if you want to use Serial monitor for debugging
  Wire.begin(5,4); // Set what pins we are being used for the gyroscope
  mpu6050.begin(); // initialise the gyroscope
  delay(200); // Pause for 200 milliseconds
  mpu6050.calcGyroOffsets(true); // Really important for the staff to NOT MOVE here, or X Y Z angles will consistantly drift
  gy = Gyro();  // create a Gyro object, which is a custom class found in gyr.cpp and gyr.h
  updateGyro(); // update the Gyro object to hold the current values for acceleration and angle

  //Set up the LEDs
  initialiseLedSet();
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  //Set up the LED display
  display.init(); 

  MENU_MODE = true;
  MENU_OPTION = 3;
}


////////////////////////
void loop() {
  timer = millis();
  detectMenu();
  if (MENU_MODE){
    displayMenu();
  }
  
  displayGyro();
  //all these menu options must loop in 100ms intervals - see within for details
  if (MENU_OPTION==1){twinkleMode();}
  else if (MENU_OPTION==2){timeMode();}
  else if (MENU_OPTION==3){rainbowMode();}
}


void updateGyro() {
    mpu6050.update();
    gy.update(mpu6050.getGyroAngleX(), mpu6050.getGyroAngleY(),mpu6050.getGyroAngleZ(),mpu6050.getAccX(),mpu6050.getAccY(),mpu6050.getAccZ());
}

////////////////////////
//
////////////////////////
void initialiseLedSet() {
  int setSize = NUM_LEDS / NUM_LED_SETS;
  for (int x=0;x<NUM_LED_SETS;x++)
  {
      CRGBSet leds1(leds(x*setSize,(x*setSize)+(setSize-1)));
      ledarray[x] = leds1;
      sizearray[x] = setSize;
  }
}

////////////////////////


uint8_t bCount = 0;
int bCountMax = 8;
int confirmCount = 0;
int confirmOption = 1;
int iterCount = 0;
int switchCount = 0;

void displayMenu() {
  //Spherical angle - use direction of gravity on Z
  float sphAng = fabs(gy.getTimePt().accZ  +0.05);
  if (sphAng<0.3){
    if (MENU_OPTION!=3){resetVariables();}
    MENU_OPTION=3;   
    }
  else if (sphAng>0.85) {
    if (MENU_OPTION!=1){resetVariables();}
    MENU_OPTION=1;     }
  else {
    if (MENU_OPTION!=2){resetVariables();}
    MENU_OPTION=2;} 

  //confirmation logic - flash every 8 iterations. if it stays the same for 5 flashes then break from menu
  if (bCount%bCountMax==0 && timer>16000){

    if (confirmOption==MENU_OPTION )  { 
      confirmCount++;
      leds.fill_solid(CHSV(0,0,0));
      FastLED.delay(100);
      } 
    else {
      confirmCount=0;
      confirmOption=MENU_OPTION;  //set variables for next loop
      display.clear();
    }

    for (int x=0;x<confirmCount;x++){ //draw on screen what confirmation count we are on
      display.drawString(10+(5*x), 50, "X");
    }
    
    if (confirmCount ==4) { 
      MENU_MODE = false; iterCount=0; switchCount = 0; //reset 'detect off count' so can be reused for 'detect menu count'
      display.clear();
    } 
  }
  bCount++;

  display.display();
}

////////////////////////
//detect menu parameters

int lastZ = 0;
bool posRotation = false;
int requiredSwitches=12;
int detectMenuTimeout = 40;
float angTol = 10;

void detectMenu() {

  if (timer <16000) {
    MENU_MODE=true;
    return;
  }
  
  //called 10x per secondq
  iterCount++;
  //make sure the staff is horizontal to get into menu mode
  float sphAng = fabs(gy.getTimePt().accZ+0.05);
  if (sphAng >0.45){switchCount=0; iterCount=0;}

  //detect the changes in spin
  int zAng = gy.getTimePt().angleZ;
  if ((zAng >lastZ+angTol) && (!posRotation)) {switchCount++; posRotation=true; iterCount=0;}
  else if  ((zAng <lastZ-angTol) && (posRotation)) {switchCount++;  posRotation=false; iterCount=0;}
  lastZ=zAng;

  //if we hit the required number of switches
  if (switchCount==requiredSwitches) {
    MENU_MODE=true; iterCount=0; switchCount = 0; //reset local variables
    bCount = 0; confirmCount = 0;}//initiailise menu variables

  //timeout menu detection if no switches detected for a while
  if (iterCount>detectMenuTimeout) {iterCount=0; switchCount = 0;}
}


////////////////////////
void displayGyro() {
  display.clear();
  String men = String(MENU_OPTION);
  display.drawString(5,3, "Setting = "+men);

  display.fillCircle(32, 15, fabs(gy.getTimePt().accX)*10);
  display.drawString(32, 28, "aX");
  display.fillCircle(64, 15, fabs(gy.getTimePt().accY)*10);
  display.drawString(64, 28, "aY");
  display.fillCircle(96, 15, fabs(gy.getTimePt().accZ)*10);
  display.drawString(96, 28, "aZ");

  display.drawString(5,35, "x");
  display.drawProgressBar(10, 40, 110, 4, ((gy.getTimePt().angleX+180)/360)*100);
  display.drawString(5,43, "y");
  display.drawProgressBar(10, 48, 110, 4, ((gy.getTimePt().angleY+180)/360)*100);
  display.drawString(5,51, "z");
  display.drawProgressBar(10, 56, 110, 4, ((gy.getTimePt().angleZ+180)/360)*100);

  display.display();
}



////////////////////////
//fill the whole thing linearly with a ranbow
//cycle rainbow forwards in colour
uint8_t hue = 0;
uint8_t delta = 10;

void rainbowMode(){
  updateGyro();
  for (int x=0; x<2; x++){
    leds.fill_rainbow(hue, delta);
    hue=hue+5;
    FastLED.delay(50);
  }
}

////////////////////////
//turn random leds on and fade them out
//colour based on position
//sudden change in acceleration means all on at full brightness
int brightArr [NUM_LEDS] = {};
int onOffArr [NUM_LEDS] = {};
int colourArr [NUM_LEDS] = {};
int twMode_gyroCount1 = 0;

void twinkleMode(){
  for (int x=0;x<5;x++){
    updateGyro();
    //count how many leds are on so we dont get stuck in a loop finding one thats off
    int numLEDsOn = 0;
    for (int i=0;i<NUM_LEDS;i++)  {numLEDsOn+=onOffArr[i];} 

    //in two scenarios, we put all the lights on
    bool allLightsActiv = false;
    if (!MENU_MODE)
    {
      //if acc changes AND there is no rotation
      if (gy.getCalcs().rangeZacc>1 && gy.getCalcs().currentXYZSpeed<25)
      { allLightsActiv = true;}
      //if spinning stops suddenly
      if (gy.getCalcs().currentZSpeed>30){twMode_gyroCount1++;}
      else { 
        if (twMode_gyroCount1>100){allLightsActiv = true;}
        twMode_gyroCount1 = 0; 
      }
    }

    //if theyre not all on already then choose how to turn on
    if (numLEDsOn<NUM_LEDS-1){ 
      // if there's a sudden change in acceleration, turn on all LEDS
      if (allLightsActiv){
        int coloursetforall = map(timer,0,12800,0,255); //std::rand()%255; 
        CHSV colour(coloursetforall, 255, 255);
        for (int a = 0; a<NUM_LEDS;a++){
          onOffArr[a]=1;
          colourArr[a] = coloursetforall; 
          brightArr[a] = 255; 
        }
      }
      else { //choose a random one to turn on
        int newLED = std::rand() % NUM_LEDS;
        while (onOffArr[newLED]==1){  
          newLED = std::rand() % NUM_LEDS;
        }
        onOffArr[newLED]=1;
        colourArr[newLED] = std::rand() % 255; //give it a colour based on z
        brightArr[newLED] = 255;
      }
    }

    //cycle through all leds, making each darker and turn off the darkest ones
    for (int a = 0; a<NUM_LEDS;a++){  
      if (onOffArr[a]==1){
        CHSV colour(colourArr[a], 255, brightArr[a]);
        leds[a]=colour;
        brightArr[a] = brightArr[a] - 2;
        if (brightArr[a]<30)  {onOffArr[a]=0; colourArr[a]=0; brightArr[a]=0; leds[a]=CRGB::Black;}
      }
    }
    FastLED.delay(20);
  }
}


////////////////////////
//change colour as a function of time (for the first bit so they sync)
//then strobe colours in dragon mode (col based on time)
//float avgSpeed = 0;
int tMode_gyroCount1 = 0;
uint8_t tMode_cycleIter = 0; 

void timeMode(){
  for (int x=0; x<2; x++){
    timer = millis();
    updateGyro();
    if (gy.getCalcs().currentZSpeed>60){tMode_gyroCount1++;}
    else {tMode_gyroCount1 = 0; tMode_cycleIter=0;}

    //normally 120, but 30 for testing
    if (tMode_gyroCount1>120 && !MENU_MODE) {
      int a = tMode_cycleIter%NUM_LED_IN_SET;
      CHSV colourCycle(map(timer,0,3200,0,255),255,255);
      if(int(floor(tMode_cycleIter/NUM_LED_IN_SET))%2==0){centre_out_dot(a, colourCycle, 100, true);}
      else {centre_out_dot(a, colourCycle, 100, false);}
      tMode_cycleIter++;
      FastLED.delay(50);
      leds.fill_solid(CHSV(0,0,0));
    }
    else {
      leds.fill_solid(CHSV(map(timer,0,12800,0,255),255,255));
      FastLED.delay(50);
    }
  }
}

////////////////////////
void centre_out_dot(uint8_t y, CRGB color, uint32_t wait, bool first) {  //Note: y is ledarray number
  for (int b=0; b<NUM_LED_SETS;b++)
  {
    if (((int)floor(b/4))%2==0){
      if (first && b%4==0){ ledarray[b][y] = color;   }
      if (!first && b%4==3) {ledarray[b][y] = color; } 
      if (first && b%4==1){ ledarray[b][NUM_LED_IN_SET-y-1] = color;   }
      if (!first && b%4==2){ ledarray[b][NUM_LED_IN_SET-y-1] = color;   }
    }
    else {
      if (first && b%4==3){ ledarray[b][NUM_LED_IN_SET-y-1] = color;   }
      if (!first && b%4==0) {ledarray[b][NUM_LED_IN_SET-y-1] = color; } 
      if (first && b%4==2){ ledarray[b][y] = color;   }
      if (!first && b%4==1){ ledarray[b][y] = color;   }
    }
  }
}  

