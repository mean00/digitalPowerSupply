/**************************************************************************/
/*! 
    @file     trianglewave.pde
    @author   Adafruit Industries
    @license  BSD (see license.txt)

    This example will generate a triangle wave with the MCP4725 DAC.   

    This is an example sketch for the Adafruit MCP4725 breakout board
    ----> http://www.adafruit.com/products/935
 
    Adafruit invests time and resources providing this open source code, 
    please support Adafruit and open-source hardware by purchasing 
    products from Adafruit!
*/
/**************************************************************************/
#include <Wire.h>
#include "Adafruit_GFX_AS.h" //
#include "Adafruit_ILI9341_STM.h" //
#include <Adafruit_MCP4725.h>
#include "ILI9341_extended.h"
ILI9341 *ili=NULL;

void mySetup(void) {
  Serial.begin(57600);
  Serial.println("Hello!");
  
  SPI.begin();
  SPI.setBitOrder(MSBFIRST); // Set the SPI bit order
  SPI.setDataMode(SPI_MODE0); //Set the  SPI data mode 0
  SPI.setClockDivider (SPI_CLOCK_DIV4); // Given for 10 Mhz...
  

  
  ili=new ILI9341(PA10 /* CS */, PA12 /* DC */, PA11 /* RST */);
  ili->begin();    
  ili->fillScreen(ILI9341_RED);
  ili->setTextColor(ILI9341_WHITE,ILI9341_BLACK);
  ili->setRotation(3);

  
}

void myLoop(void) {
}
