// #include <Arduino.h>
// #include <Wire.h>
// #include <MPU6050_tockn.h>
// #include <SSD1306Wire.h>
// #include <FastLED.h>

// void displayGyro();
// void displayLED();
// void initialiseLedSet();
// void CD77_colorwipe_dot(uint8_t y, CRGB color, uint32_t wait) ;
// void CD77_arrayfill(CRGB color, uint16_t wait);

// void SS_centre_out_all(uint8_t y, CRGB color, uint32_t wait) ;
// void SS_centre_out_all2(uint8_t y, CRGB color, uint32_t wait, bool first) ;
// void SS_centre_out_dot(uint8_t y, CRGB color, uint32_t wait, bool first) ;
// void SS_centre_out_dot_inv(uint8_t y, CRGB color, uint32_t wait, bool first) ;

// uint8_t dotX, dotY, dotZ;

// #define DATA_PIN    13
// #define LED_TYPE    WS2812B
// #define COLOR_ORDER GRB
// #define NUM_LEDS    192
// #define NUM_LED_SETS 48
// #define NUM_LED_IN_SET 4 ///must be leds/sets
// #define BRIGHTNESS  50
// #define FRAMES_PER_SECOND 120
// #define SWITCH 1 

// MPU6050 mpu6050(Wire);  // create gyro object
// SSD1306Wire display(0x3c, 5, 4);

// CRGB rawleds[NUM_LEDS];
// CRGBSet leds(rawleds, NUM_LEDS);
// struct CRGB * ledarray[NUM_LED_SETS];
// uint8_t sizearray[NUM_LED_SETS];

// uint8_t hue = 0;
// uint8_t delta = 10;
// uint8_t deltaby8 = 32;
// uint8_t fuckydelay = 100;
// int fuckycount = 0;

// void setup() {
//   Serial.begin(921600);
//   Wire.begin(5,4); // tell the I2C library what pins we are using for SCL(serial clock) SDA(serial data)
//   mpu6050.begin();
//   delay(200);
//   mpu6050.calcGyroOffsets(true);
//   initialiseLedSet();
//   FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
//   FastLED.setBrightness(BRIGHTNESS);
//   display.init(); // init OLED display
// }

// void loop() {
//   mpu6050.update();
//   displayGyro();
//   displayLED();
// }


// void displayLED() {
//   dotX = map(mpu6050.getGyroAngleX(), 0, 90, 0, 255);
//   dotY = map(mpu6050.getAngleY(), 0, 90, 0, 255);
//   dotZ = map(mpu6050.getAngleZ(), 0, 90, 0, 255);
  
//   // leds(0,NUM_LEDS/2 - 1).fill_rainbow(dotY);  // fill the first 20 items with a rainbow
//   // leds(NUM_LEDS/2, NUM_LEDS-1) = leds(NUM_LEDS/2-1,0);
//   CHSV hsv(dotX, 255, 255);

//   // CRGB rgb;
//   // hsv2rgb_spectrum(hsv, rgb);
//   // leds.fill_solid(rgb);

//   // CRGB col = ColorFromPalette(RainbowColors_p, dotX, 255, LINEARBLEND);
//   Serial.printf("dotX: %i, angleX: %f\n", dotX, mpu6050.getGyroAngleX());
  
//   if (SWITCH==1)
//   {
//     leds.fill_rainbow(hue++, delta);
//     FastLED.delay(0.003);
//    // FastLED.show();
//   }
//   else if (SWITCH==2)
//   {
//     for (int a =0;a<NUM_LED_IN_SET;a++){
//       SS_centre_out_all(a, hsv, 200);
//       FastLED.delay(200);
//       leds.fill_solid(CRGB::Black); //off for filling up
//     }
//     for (int a =NUM_LED_IN_SET-1;a>0;a--){
//       SS_centre_out_all(a, hsv, 200);
//       FastLED.delay(200);
//       leds.fill_solid(CRGB::Black); //off for filling up
//     }
//   }
//   else if (SWITCH==3)
//   {
//     for (int a =0;a<NUM_LED_IN_SET;a++){
//       SS_centre_out_all2(a, hsv, 200, true);
//       FastLED.delay(200);
//     }
//     for (int a =0;a<NUM_LED_IN_SET;a++){
//       SS_centre_out_all2(a, hsv, 200, false);
//       FastLED.delay(200);
//     }
//     leds.fill_solid(CRGB::Black);
//     FastLED.delay(200);;
//   }
//   else if (SWITCH==4)
//   {
//     for (int a =0;a<NUM_LED_IN_SET;a++){
//       CHSV colour1(hue+dotX, 255, 255);
//       SS_centre_out_dot(a, colour1, abs((int)fuckydelay-128), true);
//       hue+=deltaby8;
//       fuckydelay--;
//     }
//     for (int a =0;a<NUM_LED_IN_SET;a++){
//       CHSV colour1(hue+dotX, 255, 255);
//       SS_centre_out_dot(a, colour1, abs((int)fuckydelay-128), false);
//       hue+=deltaby8;
//       fuckydelay--;
//     }
//   }
//   else if (SWITCH==5)
//   {
//     for (int a =0;a<NUM_LED_IN_SET;a++){
//       CHSV colour1(hue+dotX, 255, 255);
//       SS_centre_out_dot_inv(a, colour1, 100, true);
//     }
//     for (int a =0;a<NUM_LED_IN_SET;a++){
//       CHSV colour1(hue+dotX, 255, 255);
//       SS_centre_out_dot_inv(a, colour1, 100, false);
//     }
//   }
// }


// /* functions to tackle sets */
// void SS_centre_out_all(uint8_t y, CRGB color, uint32_t wait) {  //Note: y is ledarray number
//   for (int b=0; b<NUM_LED_SETS;b++)
//   {
//     if (b%2==1){ ledarray[b][y] = color;   }
//     else {ledarray[b][NUM_LED_IN_SET-y-1] = color; } 
//   }
// }  

// void SS_centre_out_all2(uint8_t y, CRGB color, uint32_t wait, bool first) {  //Note: y is ledarray number
//   for (int b=0; b<NUM_LED_SETS;b++)
//   {
//     if (((int)floor(b/4))%2==0){
//       if (first && b%4==0){ ledarray[b][y] = color;   }
//       if (!first && b%4==3) {ledarray[b][y] = color; } 
//       if (first && b%4==1){ ledarray[b][NUM_LED_IN_SET-y-1] = color;   }
//       if (!first && b%4==2){ ledarray[b][NUM_LED_IN_SET-y-1] = color;   }
//     }
//     else {
//       if (first && b%4==3){ ledarray[b][NUM_LED_IN_SET-y-1] = color;   }
//       if (!first && b%4==0) {ledarray[b][NUM_LED_IN_SET-y-1] = color; } 
//       if (first && b%4==2){ ledarray[b][y] = color;   }
//       if (!first && b%4==1){ ledarray[b][y] = color;   }
//     }
//   }
// }  

// void SS_centre_out_dot_inv(uint8_t y, CRGB color, uint32_t wait, bool first) {  //Note: y is ledarray number
//   for (int b=0; b<NUM_LED_SETS;b++)
//   {
//     //if (((int)floor(b/4))%2==0){
//       if (first && b%4==0){ ledarray[b][y] = CRGB::Black;   }
//       if (!first && b%4==3) {ledarray[b][y] = CRGB::Black; } 
//       if (first && b%4==1){ ledarray[b][NUM_LED_IN_SET-y-1] = CRGB::Black;   }
//       if (!first && b%4==2){ ledarray[b][NUM_LED_IN_SET-y-1] = CRGB::Black;   }
//     // }
//     // else {
//       if (first && b%4==3){ ledarray[b][y] = CRGB::Black;   }
//       if (!first && b%4==0) {ledarray[b][y] = CRGB::Black; } 
//       if (first && b%4==2){ ledarray[b][NUM_LED_IN_SET-y-1] = CRGB::Black;   }
//       if (!first && b%4==1){ ledarray[b][NUM_LED_IN_SET-y-1] = CRGB::Black;   }
//     // }
//   }
//   FastLED.delay(wait);
//   leds.fill_rainbow(hue++, delta);
// }  

// void SS_centre_out_dot(uint8_t y, CRGB color, uint32_t wait, bool first) {  //Note: y is ledarray number
//   for (int b=0; b<NUM_LED_SETS;b++)
//   {
//     if (((int)floor(b/4))%2==0){
//       if (first && b%4==0){ ledarray[b][y] = color;   }
//       if (!first && b%4==3) {ledarray[b][y] = color; } 
//       if (first && b%4==1){ ledarray[b][NUM_LED_IN_SET-y-1] = color;   }
//       if (!first && b%4==2){ ledarray[b][NUM_LED_IN_SET-y-1] = color;   }
//     }
//     else {
//       if (first && b%4==3){ ledarray[b][NUM_LED_IN_SET-y-1] = color;   }
//       if (!first && b%4==0) {ledarray[b][NUM_LED_IN_SET-y-1] = color; } 
//       if (first && b%4==2){ ledarray[b][y] = color;   }
//       if (!first && b%4==1){ ledarray[b][y] = color;   }
//     }
//   }
//   FastLED.delay(wait);
//   leds.fill_solid(CRGB::Black);
// }  

// void CD77_colorwipe_dot(uint8_t y, CRGB color, uint32_t wait) {  //Note: y is ledarray number
//   if (y%2==0)
//   { 
//     for (uint8_t x = 0; x < sizearray[y]; x++) {
//       ledarray[y][x] = color;
//       FastLED.delay(wait);
//       ledarray[y][x] =CRGB::Black;
//     }
//   }
//   else {
//     for (int x = sizearray[y]-1; x >=0; x--) {
//       Serial.printf("Y: %i, X: %i\n", y, x);
//       ledarray[y][x] = color;
//       FastLED.delay(wait);
//       ledarray[y][x] =CRGB::Black;
//     }
//   }
  
// }  
// void CD77_arrayfill(CRGB color, uint16_t wait){
  
//    for (uint8_t x=0; x<NUM_LED_SETS; x++){
//     fill_solid(ledarray[x], sizearray[x], color);
//     FastLED.show();
//     delay(wait);
//     fill_solid( ledarray[x], sizearray[x], CRGB::Black);
//     FastLED.show();
   
//    }
// }


// /* functions that wont need editing */

// void initialiseLedSet() {
//   int setSize = NUM_LEDS / NUM_LED_SETS;
//   for (int x=0;x<NUM_LED_SETS;x++)
//   {
//       CRGBSet leds1(leds(x*setSize,(x*setSize)+(setSize-1)));
//       ledarray[x] = leds1;
//       sizearray[x] = setSize;
//   }
// }


// void displayGyro() {
//   display.clear();

//   display.fillCircle(32, 15, fabs(mpu6050.getAccX())*10);
//   display.drawString(32, 28, "aX");
//   display.fillCircle(64, 15, fabs(mpu6050.getAccY())*10);
//   display.drawString(64, 28, "aY");
//   display.fillCircle(96, 15, fabs(mpu6050.getAccZ())*10);
//   display.drawString(96, 28, "aZ");
//   // Serial.println(fabs(mpu6050.getAccZ())*10);

//   display.drawString(5,35, "x");
//   display.drawProgressBar(10, 40, 110, 4, ((mpu6050.getAngleX()+180)/360)*100);
//   display.drawString(5,43, "y");
//   display.drawProgressBar(10, 48, 110, 4, ((mpu6050.getAngleY()+180)/360)*100);
//   display.drawString(5,51, "z");
//   display.drawProgressBar(10, 56, 110, 4, ((mpu6050.getAngleZ()+180)/360)*100);

//   display.display();
// }