#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <SSD1306Wire.h>
#include <FastLED.h>
#include "main.h"

////////////////////////
void setup() {
  Serial.begin(921600);
  Wire.begin(5,4); // tell the I2C library what pins we are using for SCL(serial clock) SDA(serial data)
  mpu6050.begin();
  delay(200);
  mpu6050.calcGyroOffsets(true);
  initialiseLedSet();
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  display.init(); // init OLED display
}

////////////////////////
void loop() {
  timer = millis();
  mpu6050.update();
  if (MENU_MODE){
    displayMenu();
    FastLED.delay(100);
    resetVariables();
  }
  else {
    detectMenu();
    displayGyro();
    //all these menu options must loop in 100ms intervals - see within for details
    if (MENU_OPTION==1){timeMode();} //rainbowMode
    else if (MENU_OPTION==2){twinkleMode();}
    else if (MENU_OPTION==3){blockMode();}
  } 
}

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
void displayMenu() {
  display.clear();
  display.drawString(10, 0, "MENU");

  //Spherical angle - use direction of gravity on Z
  float sphAng = fabs(mpu6050.getAccZ()+0.1);
  if (sphAng<0.3){
    display.drawString(10, 30, "BLOCK");
    leds.fill_solid(CHSV(128,255,bBrightness));    //@@@@aqua to blue
    MENU_OPTION=3;   
    lastGyroZ = mpu6050.getGyroAngleZ(); }
  else if (sphAng>0.85) {
    display.drawString(10, 30, "RAINBOW");
    leds.fill_solid(CHSV(32,255,bBrightness));    //red to @@@@orange
    MENU_OPTION=1;     }
  else {
    display.drawString(10, 30, "TWINKLE");
    leds.fill_solid(CHSV(244,255,bBrightness));    //purple to @@@@pink
    MENU_OPTION=2;
    lastAccZtwink = mpu6050.getAccZ(); } //initiate zacc fo twink detecting change  
  //in the above we could go more advanced so each gradient applies centre out??

  //confirmation logic - flash every 8 iterations. if it stays the same for 5 flashes then break from menu
  if (bCount%bCountMax==0){
    bBrightness=0;

    if (confirmOption==MENU_OPTION )  { confirmCount++;} //&& abs(confirmBrightness-global_bri)<30) 
    else {
      confirmCount=0;
      confirmOption=MENU_OPTION;  //set variables for next loop
      //confirmBrightness = global_bri;
    }

    for (int x=0;x<confirmCount;x++){ //draw on screen what confirmation count we are on
      display.drawString(10+(5*x), 50, "X");
    }
    
    if (confirmCount ==5) { 
      MENU_MODE = false; iterCount=0; switchCount = 0; //reset 'detect off count' so can be reused for 'detect menu count'
      leds.fill_solid(CHSV(0,0,0));
    } 
  }
  else {bBrightness=255;}
  bCount++;

//note: need to make sure variuabkles set to base when menui activated??
  display.display();
}

////////////////////////
void detectMenu() {
  //called 10x per second
  iterCount++;
  //make sure the staff is upright to get into menu mode
  float sphAng = fabs(mpu6050.getAccZ()+0.1);
  if (sphAng <0.75){switchCount=0; iterCount=0;}

  //detect the changes in spin
  int zAng = mpu6050.getGyroAngleZ();
  if ((zAng >lastZ+angTol) && (!posRotation)) {switchCount++; posRotation=true; iterCount=0;}
  else if  ((zAng <lastZ-angTol) && (posRotation)) {switchCount++;  posRotation=false; iterCount=0;}
  lastZ=zAng;

  //if we hit the required number of switches
  if (switchCount==requiredSwitches) {
    MENU_MODE=true; iterCount=0; switchCount = 0; //reset local variables
    bCount = 0; confirmCount = 0; menuCount = 0;}//initiailise menu variables

  //timeout menu detection if no switches detected for a while
  if (iterCount>detectMenuTimeout) {iterCount=0; switchCount = 0;}
}


////////////////////////
void displayGyro() {
  display.clear();
  String men = String(MENU_OPTION);
  display.drawString(5,3, "Setting = "+men);

  display.fillCircle(32, 15, fabs(mpu6050.getAccX())*10);
  display.drawString(32, 28, "aX");
  display.fillCircle(64, 15, fabs(mpu6050.getAccY())*10);
  display.drawString(64, 28, "aY");
  display.fillCircle(96, 15, fabs(mpu6050.getAccZ())*10);
  display.drawString(96, 28, "aZ");
  // Serial.println(fabs(mpu6050.getAccZ())*10);

  display.drawString(5,35, "x");
  display.drawProgressBar(10, 40, 110, 4, ((mpu6050.getGyroAngleX()+180)/360)*100);
  display.drawString(5,43, "y");
  display.drawProgressBar(10, 48, 110, 4, ((mpu6050.getGyroAngleY()+180)/360)*100);
  display.drawString(5,51, "z");
  display.drawProgressBar(10, 56, 110, 4, ((mpu6050.getGyroAngleZ()+180)/360)*100);

  display.display();
}

////////////////////////
//change colour as a function of time (for the first bit so they sync)
//then change colour based on rotation speed, over a certain value
uint8_t lastCol = 0;
void timeMode(){
  for (int x=0; x<2; x++){
    loopNum++;
    //for the first 5 seconds just change the colour based on time
    if (loopNum<100) {
      timer = millis();
      uint8_t modTimer = map(timer,0,12800,0,255);
      CHSV solidCol(modTimer,255,255);
      leds.fill_solid(solidCol);
      lastCol = modTimer;
    }
    //After 5s, increment colour based on rotation speed
    else {
      float gyroSpeed = fabs(fabs(lastGyroZ)- fabs(mpu6050.getGyroAngleZ()));
      if (gyroSpeed>10){bMode_gyroCount++;}
      else {bMode_gyroCount = 0; lastGyroZ = 0;}

      if (bMode_gyroCount>bMode_gyroActivateCount) {
        lastCol = lastCol + 4;
        //lastCol = lastCol + floor((gyroSpeed/10)+baseIncrement);
      }
      else { 
        lastCol = lastCol++;
      }

      CHSV solidCol(lastCol,255,255);
      leds.fill_solid(solidCol);
      lastGyroZ = (2*lastGyroZ+mpu6050.getGyroAngleZ())/3;
    } 
    FastLED.delay(50);
  }
}

////////////////////////
//fill the whole thing linearly with a ranbow
//cycle rainbow forwards in colour
void rainbowMode(){
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
void twinkleMode(){
  for (int x=0;x<5;x++){
    //count how many leds are on so we dont get stuck in a loop finding one thats off
    int numLEDsOn = 0;
    for (int i=0;i<NUM_LEDS;i++)  {numLEDsOn+=onOffArr[i];} 

    //if theyre not all on already then choose how to turn on
    if (numLEDsOn<NUM_LEDS-1){ 
      // if there's a sudden change in acceleration, turn on all LEDS
      if (fabs(lastAccZtwink - mpu6050.getAccZ())>0.6 ){
        int coloursetforall = std::rand()%255; 
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
    lastAccZtwink = (2*lastAccZtwink + mpu6050.getAccZ())/3; //apply some low pass filter as we're recalculating every 0.1s

    FastLED.delay(20);
  }
}

////////////////////////
//block colour
//strobe on lights in and out on each face - after rpm over <X< for Y seconds
//colour based on position
void blockMode(){
  uint8_t symZcol = map(mpu6050.getGyroAngleX(),0,540,0,255);
  uint8_t randcol = std::rand() % 255;
  CHSV colourS(symZcol,255,255);
  CHSV colourR(randcol,255,255);
  

  float gyroSpeed = fabs(lastGyroZ - mpu6050.getGyroAngleZ());
  if (gyroSpeed>20){bMode_gyroCount++;}
  else {bMode_gyroCount = 0;}

  if (bMode_gyroCount>bMode_gyroActivateCount) {
    //leds.fill_solid(CHSV(0,0,0));
    int a = bMode_cycleIter%NUM_LED_IN_SET;
    if(int(floor(bMode_cycleIter/NUM_LED_IN_SET))%2==0){centre_out_dot(a, colourR, 100, true);}
    else {centre_out_dot(a, colourR, 100, false);}
    bMode_cycleIter++;
  }
  else {leds.fill_solid(colourS); }
  

  lastGyroZ = (2*lastGyroZ+mpu6050.getGyroAngleZ())/3;
  //lastGyroZ = mpu6050.getGyroAngleZ();
  FastLED.delay(100);
  leds.fill_solid(CHSV(0,0,0));
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
