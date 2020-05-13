#include "main.h"

////////////////////////
// This simplified code for displaying coloured LEDs and accelerometer info
// 
// It requires the hardware:
// Microcontroller (chip that's in control) = ESP32 (wemos lolin32 oled)
// 3.2m of LEDs (individually addressible LED) = WS2812b (60 per m)
// Accelerometer (detects angle and acceleration) = MPU6050
// 2x 18650 batteries
// 2x battery holders
// usb charging station (preferable)
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
// This code has the general structure:
// setup() initialises everything (when first turned on)
// loop() is called constantly
// rainbowMode() shows a simple cycle of colours which display over the rangle of LEDs
//
// NOTE: the header file main.h contains some variables called from this file, which work because of the #include above
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

  //Set up the LEDs
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  //Set up the LED display on the ESP32
  display.init(); 
}


////////////////////////
// Keeps getting called once its been ran through
////////////////////////
bool MODE = 1;

void loop() {
  mpu6050.update();   //get the new values from the accelerometer
  displayGyro();      //display on the microcontrollers LED display the acceleration and angle
  if (MODE==1){       //simple menu option
    solidMode();      //display a solid colour accross all LEDs, changing with Z rotation
  }
  else {
    rainbowMode();    //display constantly changing colours across the LEDs
  }
}



////////////////////////
void displayGyro() {
  //clear the last data on the LED display so previous values dont overlap
  display.clear();

  // Show accelerations (mostly due to the effect of gravity) as circles.
  // Bigger the circle, the more that axis is in line with gravity, or faster its spinning
  display.fillCircle(32, 15, fabs(mpu6050.getAccX())*10);
  display.drawString(32, 28, "aX");
  display.fillCircle(64, 15, fabs(mpu6050.getAccY())*10);
  display.drawString(64, 28, "aY");
  display.fillCircle(96, 15, fabs(mpu6050.getAccZ())*10);
  display.drawString(96, 28, "aZ");

  // Show the current rotation of the gyro around the X Y and Z axes
  // This starts in the middle of each bar when the gyro has not moved
  display.drawString(5,35, "x");
  display.drawProgressBar(10, 40, 110, 4, ((mpu6050.getAngleX()+180)/360)*100);
  display.drawString(5,43, "y");
  display.drawProgressBar(10, 48, 110, 4, ((mpu6050.getAngleY()+180)/360)*100);
  display.drawString(5,51, "z");
  display.drawProgressBar(10, 56, 110, 4, ((mpu6050.getAngleZ()+180)/360)*100);

  //show all of the above on the display
  display.display();
}



////////////////////////
//fill the whole set of LEDs linearly with a rainbow, changing with time
////////////////////////
uint8_t hue = 0;
uint8_t delta = 5;
void rainbowMode() {
  leds.fill_rainbow(hue, delta);  //set the led colours using the fill_rainbow function
                                  //fill_solid and fill_gradient are also food functions to use
  hue=hue+5;                      //add 5 to the hue so it cycles through the colours
  FastLED.delay(50);              //pause for 50 miliseconds so the lights dont change at warp speed
}


////////////////////////
//fill all LEDs with the same colour, which changes with Z rotation
////////////////////////
void solidMode() {
  int colourVal = map(mpu6050.getAngleZ(),0,360,0,255); //map the z rotation values from degrees to 8bit
  CHSV colour(colourVal, 255, 255);       //create a colour object (hue, saturation, brightness)
  leds.fill_solid(colour);                //set this colour on all LEDs
  FastLED.delay(100);                      //pause for 100 miliseconds so the lights dont change at warp speed
}
