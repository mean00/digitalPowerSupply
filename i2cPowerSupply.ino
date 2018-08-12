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
#include "myMCP4725.h"
#include "ILI9341_extended.h"

extern void mySetup();
extern void myLoop();

void setup(void) {
    mySetup();
  
}

void loop(void) {
    myLoop();
}
