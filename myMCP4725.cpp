/**************************************************************************/
/*! 
    @file     Adafruit_MCP4725.cpp
    @author   K.Townsend (Adafruit Industries)
	@license  BSD (see license.txt)
	
	I2C Driver for Microchip's MCP4725 I2C DAC

	This is a library for the Adafruit MCP4725 breakout
	----> http://www.adafruit.com/products/935
		
	Adafruit invests time and resources providing this open source code, 
	please support Adafruit and open-source hardware by purchasing 
	products from Adafruit!

	@section  HISTORY

    v1.0 - First release
*/
/**************************************************************************/
#include <Wire.h>
#include "myMCP4725.h"

/**************************************************************************/
/*! 
    @brief  Instantiates a new MCP4725 class
*/
/**************************************************************************/
myMCP4725::myMCP4725(WireBase &wire,int i2c) :   _i2caddr(i2c),_wire(wire)
{
  
}

 
/**************************************************************************/
/*! 
    @brief  Sets the output voltage to a fraction of source vref.  (Value
            can be 0..4095)

    @param[in]  output
                The 12-bit value representing the relationship between
                the DAC's input voltage and its output voltage.
    @param[in]  writeEEPROM
                If this value is true, 'output' will also be written
                to the MCP4725's internal non-volatile memory, meaning
                that the DAC will retain the current voltage output
                after power-down or reset.
*/
/**************************************************************************/
void myMCP4725::setVoltage( int output )
{
  _wire.beginTransmission(_i2caddr);
  _wire.write(MCP4726_CMD_WRITEDAC);
  _wire.write(output / 16);                   // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
  _wire.write((output % 16) << 4);            // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
  _wire.endTransmission();
}
/**
 * 
 * @param output
 * @param writeEEPROM
 */
void myMCP4725::setDefaultValue( int output )
{
  _wire.beginTransmission(_i2caddr);
  _wire.write(MCP4726_CMD_WRITEDACEEPROM);
  _wire.write(output / 16);                   // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
  _wire.write((output % 16) << 4);            // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
  _wire.endTransmission();
}

