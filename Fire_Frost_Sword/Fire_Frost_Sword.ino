/*
/// @file    Fire_Frost_Sword.ino
/// @brief   Simple one-dimensional fire animation with a programmable color palette
/// @example Fire2012WithPalette.ino
/// @authors Zizhong Liu & Nathan Vrubel.

 * References:
 * https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
 * https://www.instructables.com/Neopixel-Ws2812-Rainbow-LED-Glow-With-M5stick-C-Ru/
 * enable SPI bus for best performance https://github.com/FastLED/FastLED/pull/1047
 * Predefined colors: https://github.com/FastLED/FastLED/wiki/Pixel-reference#predefined-colors-list

/// @ equipments: NOTE--the data-pin on the LED lights is connected to G26 on the M5Stick
 *  https://www.amazon.com/gp/product/B08VGST8LJ
 *  https://www.amazon.com/gp/product/B06XD72LYM
 *  MIT License Laudix 2023  https://gist.github.com/LaudixGit/2c532a9aa9ad9af3dd0c315673318556
*/

#define LED_PIN     26 // //this is the only IO pin used in this example (master out, slave in)
#define COLOR_ORDER_Fire RGB
#define COLOR_ORDER_Ice BRG
#define COLOR_ORDER_Wood GBR
#define CHIPSET     WS2811
#define NUM_LEDS    50 // quantity of LEDs on you LED strand
#define BRIGHTNESS_Base  10 // how bright are the lights 255=full power
#define FRAMES_PER_SECOND 60

#define HSPI_MISO   27    //unused, for completenes only    
#define HSPI_SCLK   25    //unused, for completenes only
#define HSPI_SS     32    //unused, for completenes only
#define FASTLED_ALL_PINS_HARDWARE_SPI  //set this before including the FastLED library to us SPI
#define FASTLED_ESP32_SPI_BUS HSPI     //set this before including the FastLED library to use the 2nd SPI bus (the default, VSPI, is used by WiFi)

#include <SPI.h>           //include the SPI library
#include <M5StickCPlus.h>  //include the M5 library; this simplifies interacting with instruments on the M5
#include <FastLED.h>       //Include the library optimized to control the LED strand(s)
#include <math.h>
#include "esp_pm.h"

bool gReverseDirection = false;
float BRIGHTNESS;

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
float acc_ave = 0.0F;

float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;
float gyro_ave = 0.0F;

float pitch = 0.0F;
float roll  = 0.0F;
float yaw   = 0.0F;

static float temp = 0;

CRGB leds[NUM_LEDS];
CRGBPalette16 gPal = HeatColors_p;


void setup() {
  
  // enable the M5 and write to its LCD screen
    M5.begin();// Init M5StickC Plus.  初始化 M5StickC Plus             
    M5.Imu.Init();          // Init IMU.  初始化IMU 
    M5.Lcd.setRotation(3);  // Rotate the screen. 将屏幕旋转
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(15, 30);
    M5.Lcd.println("  gyroX    Y      Z      Ave");
    M5.Lcd.setCursor(15, 50);
    M5.Lcd.println("  acceX    Y      Z      Ave");    
    M5.Lcd.setCursor(15, 70);
    M5.Lcd.println("  Pitch   Roll    Yaw");

}


void loop() {

    DisplayFire();
    DisplayIce();
    DisplayWood();

    M5.update();
    delay(50);

}

void checkAXPPress() {
    if (M5.Axp.GetBtnPress()) {
        do {
            delay(20);
        } while (M5.Axp.GetBtnPress());

        M5.Beep.mute();
        ESP.restart();
    }
}

void DisplayFire()
{
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER_Fire>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    while ((!M5.BtnA.isPressed()) && (!M5.BtnB.isPressed())) {
        
        M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
        M5.IMU.getAccelData(&accX, &accY, &accZ);
        M5.IMU.getAhrsData(&pitch, &roll, &yaw);
        M5.IMU.getTempData(&temp);

        gyro_ave = sqrt(pow(gyroX,2)+pow(gyroY,2)+pow(gyroZ,2));
        acc_ave = sqrt(pow(accX,2)+pow(accY,2)+pow(accZ,2));
        BRIGHTNESS = BRIGHTNESS_Base * (1+sqrt(pow(gyro_ave,2)+pow(acc_ave,2))/5);

        // Add entropy to random number generator; we use a lot of it.
        random16_add_entropy( random());

        Fire2012WithPalette(); // run simulation frame, using palette colors
        FastLED.setBrightness( BRIGHTNESS );
        FastLED.show(); // display this frame
        FastLED.delay(1000 / FRAMES_PER_SECOND);    

        M5.Lcd.setCursor(50, 15);  // set the cursor location.  设置光标位置
        M5.Lcd.println("Fire: Gyro/Accel/Ahrs");
        M5.Lcd.setCursor(15, 40);
        M5.Lcd.printf(" %5.2f  %5.2f  %5.2f  %5.2f", gyroX, gyroY, gyroZ, gyro_ave);
        M5.Lcd.setCursor(210, 40);
        M5.Lcd.print(" o/s");
        M5.Lcd.setCursor(15, 60);
        M5.Lcd.printf(" %5.2f  %5.2f  %5.2f  %5.2f", accX, accY, accZ, acc_ave);
        M5.Lcd.setCursor(210, 60);
        M5.Lcd.print(" G");
        M5.Lcd.setCursor(15, 80);
        M5.Lcd.printf(" %5.2f   %5.2f   %5.2f   ", pitch, roll, yaw);
        M5.Lcd.setCursor(15, 95);
        M5.Lcd.printf("  Temperature : %.2f C", temp);
        M5.Lcd.setCursor(15, 110);
        M5.Lcd.printf("  BRIGHTNESS : %.2f  ", BRIGHTNESS);

        delay(10);
        M5.update();
        checkAXPPress();
    }

    while ((M5.BtnA.isPressed()) || (M5.BtnB.isPressed())) {
        fill_solid(leds,NUM_LEDS, 0);
        FastLED.show();
        delay(1000);
        FastLED.clearData();
        FastLED.clear(true);
        FastLED.show();
        M5.update();
        checkAXPPress();  
        M5.Beep.tone(4000);
        delay(10);
    }
    delay(50);
    M5.Beep.mute();

}

void DisplayIce()
{   
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER_Ice>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    while ((!M5.BtnA.isPressed()) && (!M5.BtnB.isPressed())) {
        
        M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
        M5.IMU.getAccelData(&accX, &accY, &accZ);
        M5.IMU.getAhrsData(&pitch, &roll, &yaw);
        M5.IMU.getTempData(&temp);

        gyro_ave = sqrt(pow(gyroX,2)+pow(gyroY,2)+pow(gyroZ,2));
        acc_ave = sqrt(pow(accX,2)+pow(accY,2)+pow(accZ,2));
        BRIGHTNESS = BRIGHTNESS_Base * (1+sqrt(pow(gyro_ave,2)+pow(acc_ave,2))/5);

        // Add entropy to random number generator; we use a lot of it.
        random16_add_entropy( random());
        Ice2012WithPalette(); // run simulation frame, using palette colors
        FastLED.setBrightness( BRIGHTNESS );
        FastLED.show(); // display this frame
        FastLED.delay(1000 / FRAMES_PER_SECOND);    

        M5.Lcd.setCursor(50, 15);  // set the cursor location.  设置光标位置
        M5.Lcd.println("Ice: Gyro/Accel/Ahrs");
        M5.Lcd.setCursor(15, 40);
        M5.Lcd.printf(" %5.2f  %5.2f  %5.2f  %5.2f", gyroX, gyroY, gyroZ, gyro_ave);
        M5.Lcd.setCursor(210, 40);
        M5.Lcd.print(" o/s");
        M5.Lcd.setCursor(15, 60);
        M5.Lcd.printf(" %5.2f  %5.2f  %5.2f  %5.2f", accX, accY, accZ, acc_ave);
        M5.Lcd.setCursor(210, 60);
        M5.Lcd.print(" G");
        M5.Lcd.setCursor(15, 80);
        M5.Lcd.printf(" %5.2f   %5.2f   %5.2f   ", pitch, roll, yaw);
        M5.Lcd.setCursor(15, 95);
        M5.Lcd.printf("  Temperature : %.2f C", temp);
        M5.Lcd.setCursor(15, 110);
        M5.Lcd.printf("  BRIGHTNESS : %.2f  ", BRIGHTNESS);

        delay(10);
        M5.update();
        checkAXPPress();
    }

    while ((M5.BtnA.isPressed()) || (M5.BtnB.isPressed())) {
        fill_solid(leds,NUM_LEDS, 0);
        FastLED.show();
        delay(1000);
        FastLED.clearData();
        FastLED.clear(true);
        FastLED.show();
        M5.update();
        checkAXPPress();
        // fill_solid(leds, NUM_LEDS, CRGB::DarkSlateGray);   
        M5.Beep.tone(4000);
        delay(10);
    }
    delay(50);
    M5.Beep.mute();

}

void DisplayWood()
{
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER_Wood>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    while ((!M5.BtnA.isPressed()) && (!M5.BtnB.isPressed())) {
        
        M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
        M5.IMU.getAccelData(&accX, &accY, &accZ);
        M5.IMU.getAhrsData(&pitch, &roll, &yaw);
        M5.IMU.getTempData(&temp);

        gyro_ave = sqrt(pow(gyroX,2)+pow(gyroY,2)+pow(gyroZ,2));
        acc_ave = sqrt(pow(accX,2)+pow(accY,2)+pow(accZ,2));
        BRIGHTNESS = BRIGHTNESS_Base * (1+sqrt(pow(gyro_ave,2)+pow(acc_ave,2))/5);

        // Add entropy to random number generator; we use a lot of it.
        random16_add_entropy( random());
        Wood2012WithPalette(); // run simulation frame, using palette colors
        FastLED.setBrightness( BRIGHTNESS );
        FastLED.show(); // display this frame
        FastLED.delay(1000 / FRAMES_PER_SECOND);    

        M5.Lcd.setCursor(50, 15);  // set the cursor location.  设置光标位置
        M5.Lcd.println("Wood: Gyro/Accel/Ahrs");
        M5.Lcd.setCursor(15, 40);
        M5.Lcd.printf(" %5.2f  %5.2f  %5.2f  %5.2f", gyroX, gyroY, gyroZ, gyro_ave);
        M5.Lcd.setCursor(210, 40);
        M5.Lcd.print(" o/s");
        M5.Lcd.setCursor(15, 60);
        M5.Lcd.printf(" %5.2f  %5.2f  %5.2f  %5.2f", accX, accY, accZ, acc_ave);
        M5.Lcd.setCursor(210, 60);
        M5.Lcd.print(" G");
        M5.Lcd.setCursor(15, 80);
        M5.Lcd.printf(" %5.2f   %5.2f   %5.2f   ", pitch, roll, yaw);
        M5.Lcd.setCursor(15, 95);
        M5.Lcd.printf("  Temperature : %.2f C", temp);
        M5.Lcd.setCursor(15, 110);
        M5.Lcd.printf("  BRIGHTNESS : %.2f  ", BRIGHTNESS);

        delay(10);
        M5.update();
        checkAXPPress();
    }

    while ((M5.BtnA.isPressed()) || (M5.BtnB.isPressed())) {
        fill_solid(leds,NUM_LEDS, 0);
        FastLED.show();
        delay(1000);
        FastLED.clearData();
        FastLED.clear(true);
        FastLED.show();
        M5.update();
        checkAXPPress();
        // fill_solid(leds, NUM_LEDS, CRGB::DarkSlateGray);   
        M5.Beep.tone(4000);
        delay(10);
    }
    delay(50);
    M5.Beep.mute();

}


// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120


void Fire2012WithPalette()
{
// Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      uint8_t colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}


void Ice2012WithPalette()
{
// Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      uint8_t colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}

void Wood2012WithPalette()
{
// Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      uint8_t colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}


