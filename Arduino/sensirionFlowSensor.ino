/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*******************************************************************************
 * Purpose: Based on Example code for the I2C communication with Sensirion
 *          Liquid Flow Sensors
 *
 *          Reads the flow measurement from the sensor and
 *          displays it on the LCD
 ******************************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include "cajalLogo.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Set up display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -----------------------------------------------------------------------------
// Sensor specific settings and variables, adjust if needed:
// -----------------------------------------------------------------------------

const int ADDRESS = 0x08; // Address for SLF3x Liquid Flow Sensors
const float SCALE_FACTOR_FLOW = 500; // Scale Factor for flow rate measurement (datasheet)
const float SCALE_FACTOR_TEMP = 200.0; // Scale Factor for temperature measurement (datasheet)
const char *UNIT_FLOW = "ml/min"; //physical unit of the flow rate measurement
const char *UNIT_TEMP = "C"; //physical unit of the temperature measurement


int ret;
uint16_t aux_value;
uint16_t sensor_flow_value;
uint16_t sensor_temp_value;
int16_t signed_flow_value;
int16_t signed_temp_value;
float scaled_flow_value;
float scaled_temp_value;
byte aux_crc;
byte sensor_flow_crc;
byte sensor_temp_crc;

bool airInLine = false;
bool fastFlow = false;
bool airLatch = false;
bool fastLatch = false;

// Serial variables
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean inputOK = false;
boolean tState = false;

// -----------------------------------------------------------------------------
// Arduino setup routine, just runs once:
// -----------------------------------------------------------------------------
void setup() {


  Serial.begin(115200); // initialize serial communication


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
  //  Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setTextColor(SSD1306_WHITE);

  drawSplash();

  delay(1000);


  
  Wire.begin();       // join i2c bus (address optional for master)

  do {
    // Soft reset the sensor
    Wire.beginTransmission(0x00);
    Wire.write(0x06);
    ret = Wire.endTransmission();
    if (ret != 0) {
      Serial.println("Error while sending soft reset command, retrying...");
      delay(500); // wait long enough for chip reset to complete
    }
  } while (ret != 0);

  delay(50); // wait long enough for chip reset to complete

  Wire.beginTransmission(ADDRESS);
  Wire.write(0x36);
  Wire.write(0x08);
  ret = Wire.endTransmission();
  if (ret != 0) {
    Serial.println("Error during write measurement mode command");

  } else {
    delay(1000);
  }

}

// -----------------------------------------------------------------------------
// The Arduino loop routine runs over and over again forever:
// -----------------------------------------------------------------------------
void loop() {

  if (stringComplete) {
    char firstChar = inputString[0];
    switch (firstChar){

      case ('B') : 
        // Is there a bubble since last ask?
        Serial.println(airLatch);
        airLatch = false;
        break;

      case ('W') : 
        // is there a flow fault since last ask?
        Serial.println(fastLatch);
        fastLatch = false;
        break;

      case ('F') : 
        // Return current flow rate in mL/min
        Serial.println(scaled_flow_value);
        break;

      case ('T') : 
       // Return current temperature in deg C
       Serial.println(scaled_temp_value);
        break;

      case ('Y') : 
        // Return ID string
        Serial.println(F("flowSensor"));
        break;
    }
  }

  inputString = "";
  stringComplete = false;


  // Read from sensor 

      Wire.requestFrom(ADDRESS, 9);

      sensor_flow_value  = Wire.read() << 8; // read the MSB from the sensor
      sensor_flow_value |= Wire.read();      // read the LSB from the sensor
      sensor_flow_crc    = Wire.read();
      sensor_temp_value  = Wire.read() << 8; // read the MSB from the sensor
      sensor_temp_value |= Wire.read();      // read the LSB from the sensor
      sensor_temp_crc    = Wire.read();
      aux_value          = Wire.read() << 8; // read the MSB from the sensor
      aux_value         |= Wire.read();      // read the LSB from the sensor
      aux_crc            = Wire.read();


      signed_flow_value = (int16_t) sensor_flow_value;
      scaled_flow_value = ((float) signed_flow_value) / SCALE_FACTOR_FLOW;
      signed_temp_value = (int16_t) sensor_temp_value;
      scaled_temp_value = ((float) signed_temp_value) / SCALE_FACTOR_TEMP;


  // Display
    drawFlowData(scaled_flow_value, scaled_temp_value, aux_value);
      
    

  }



void drawSplash(void) {

  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - SCREEN_WIDTH ) / 2,
    (display.height() - SCREEN_HEIGHT) / 2,
    logo_bmp, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
}

void drawFlowData(float value, float temp, byte flags){
  
  display.clearDisplay();
  
  /* Naive display.print() will jump all over the place if there's a 
   * negative value or |value| < 10 ml
   * 
   * Add in some logic to help
   */

  int flowSpace = 0;
  if (abs(value) < 10.0){
    flowSpace += 19;
  }

  if (value >= 0.0) {
    flowSpace += 12;
  }

  display.drawLine(10, 32, 112, 32, SSD1306_WHITE);
  display.drawLine(52, 38, 52, 55, SSD1306_WHITE);
  
  display.setCursor(flowSpace+0, 25);

  display.setTextSize(1);
  display.setFont(&FreeSans18pt7b);
  display.println(value);
  
  display.setCursor(104, 12);
  display.setFont();
  display.setTextSize(1);
  display.println("mL/");
  display.setCursor(104, 17);
  display.println("min");

  /*
  display.setTextSize(1);
  display.setCursor(0, 32);
  display.print(temp);
  display.println(UNIT_TEMP);
  */
  
  display.setCursor(10, 40);
  //display.print(flags, HEX);
  // 0 - Air in line
  // 1 - high flow
  // 2-4 Unused
  // 5 - Exp smoothing
  // 6-15 unused
  airInLine = bitRead(flags, 0);
  fastFlow = bitRead(flags, 1);

  display.setFont(&FreeSans12pt7b);
  display.setCursor(0, 56);
  display.print("Air");
  if (airInLine){
    display.fillRect(34, 42, 12, 12, SSD1306_WHITE);
    airLatch = true;
  }
  else{
    display.drawRect(34, 42, 12, 12, SSD1306_WHITE);
  }

  display.setCursor(60, 56);
  display.print("Flow");
  if (fastFlow){
    display.fillRect(114, 42, 12, 12, SSD1306_WHITE);
    fastLatch = true;
  }
  else{
    display.drawRect(114, 42, 12, 12, SSD1306_WHITE);
  }


  
  display.display();
}

void serialEvent() {

  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
      //Serial.println("Input:");
    /*  Serial.println(inputString);
      Serial.println("Size:");
      Serial.println(inputString.length()-1);
      */
    }
  }
}
