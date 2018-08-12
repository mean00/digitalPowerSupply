/**************************************************************************/
/*! 
    @file     Adafruit_MCP4725.h
    @author   K. Townsend (Adafruit Industries)
	@license  BSD (see license.txt)
	
	This is a library for the Adafruit MCP4725 breakout board
	----> http://www.adafruit.com/products/935
	
	Adafruit invests time and resources providing this open source code, 
	please support Adafruit and open-source hardware by purchasing 
	products from Adafruit!

	@section  HISTORY

    v1.0  - First release
*/
/**************************************************************************/

#include <Wire.h>

#define MCP4726_CMD_WRITEDAC            (0x40)  // Writes data to the DAC
#define MCP4726_CMD_WRITEDACEEPROM      (0x60)  // Writes data to the DAC and the EEPROM (persisting the assigned value after reset)
/**
 */
class myMCP4725
{
 public:
            myMCP4725(WireBase &wire,int address);
    void    setVoltage( int output);
    void    setDefaultValue( int output );

 private:
  const uint8_t _i2caddr;
        WireBase &_wire;
};
