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
  //display.clear();
  //display.drawString(10, 0, "MENU");

  //Spherical angle - use direction of gravity on Z
  float sphAng = fabs(mpu6050.getAccZ()+0.1);
  if (sphAng<0.3){
    //display.drawString(10, 30, "BLOCK");
    //leds.fill_solid(CHSV(128,255,bBrightness));    //@@@@aqua to blue
    if (MENU_OPTION!=3){resetVariables();}
    MENU_OPTION=3;   
    }
  else if (sphAng>0.85) {
    //display.drawString(10, 30, "RAINBOW");
    //leds.fill_solid(CHSV(32,255,bBrightness));    //red to @@@@orange
    if (MENU_OPTION!=1){resetVariables();}
    MENU_OPTION=1;     }
  else {
    //display.drawString(10, 30, "TWINKLE");
    //leds.fill_solid(CHSV(244,255,bBrightness));    //purple to @@@@pink
    MENU_OPTION=2;
    lastAccZtwink = mpu6050.getAccZ(); } //initiate zacc fo twink detecting change  
  //in the above we could go more advanced so each gradient applies centre out??

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

//note: need to make sure variuabkles set to base when menui activated??
  display.display();
}

////////////////////////
void detectMenu() {

  if (timer <16000) {
    MENU_MODE=true;
    return;
  }
  
  //called 10x per secondq
  iterCount++;
  //make sure the staff is horizontal to get into menu mode
  float sphAng = fabs(mpu6050.getAccZ()+0.1);
  if (sphAng >0.3){switchCount=0; iterCount=0;}

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
      if (fabs(lastAccZtwink - mpu6050.getAccZ())>0.6 && !MENU_MODE){
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
    lastAccZtwink = (2*lastAccZtwink + mpu6050.getAccZ())/3; //apply some low pass filter as we're recalculating every 0.1s

    FastLED.delay(20);
  }
}


////////////////////////
//change colour as a function of time (for the first bit so they sync)
//then strobe colours in dragon mode (col based on time)
//float avgSpeed = 0;
void timeMode(){
  for (int x=0; x<2; x++){
    timer = millis();
    float gyroSpeed = fabs(lastGyroZ - mpu6050.getGyroAngleZ());

    if (gyroSpeed>20){tMode_gyroCount1++;}
    else {tMode_gyroCount1 = 0; bMode_cycleIter=0;}

    //normally 120, but 30 for testing
    if (tMode_gyroCount1>120 && !MENU_MODE) {
      int a = bMode_cycleIter%NUM_LED_IN_SET;
      CHSV colourCycle(map(timer,0,3200,0,255),255,255);
      if(int(floor(bMode_cycleIter/NUM_LED_IN_SET))%2==0){centre_out_dot(a, colourCycle, 100, true);}
      else {centre_out_dot(a, colourCycle, 100, false);}
      bMode_cycleIter++;
      FastLED.delay(50);
      leds.fill_solid(CHSV(0,0,0));
    }
    else {
      leds.fill_solid(CHSV(map(timer,0,12800,0,255),255,255));
      FastLED.delay(50);
    }
  
    lastGyroZ = ((2*lastGyroZ)+mpu6050.getGyroAngleZ())/3;
    //avgSpeed = ((2*avgSpeed)+gyroSpeed)/3;
  }
}

////////////////////////
//DEPRICATED
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
