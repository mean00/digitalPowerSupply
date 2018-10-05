/***************************************************
 Digitally controlled power supply
 *  * GPL v2
 * (c) mean 2018 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "SPI.h"
#include "ILI9341_extended.h"
#include "adc.h"
#include "Fonts/digitLcd56.h"
#include "Fonts/Targ56.h"
#include "Fonts/FreeSans18pt7b.h"
#include "Fonts/FreeSans9pt7b.h"
#include "simplerMCP4725.h"
#include "SoftWire.h"
#include "Wire.h"
#include "simpler_INA219.h" // Add dummy comment to make arduino cmake not consider this line
#include "MapleFreeRTOS1000.h"
#include "MapleFreeRTOS1000_pp.h"
#include "mcp23017.h"
#include "MCP23017_rtos.h"
#include "dacRotaryControl.h"

// ILI9341 is using HW SPI + those pins
// HW SPI= pin                  PA5/PA6/PA7
#define TFT_DC                  PA8 
#define TFT_RST                 PA9 
#define TFT_CS                  PA10
//
#define MCP23017_INTERRUPT_PIN  PA2
#define MCP23017_RESET_PIN      PA1
// I2C MAPPING
// HW I2C = pins PB6/PB7

#define VOLTAGE_ADC_I2C_ADR     0x61
#define MAXCURRENT_ADC_I2C_ADR  0x60
#define MCP23017_I2C_ADDR       0x00 // Actual =0x20+0
#define INA219_I2C_ADDR         0x40

// Our globals

xMutex              *i2cMutex; // to protect i2c from concurrent accesses

ILI9341             *tft=NULL;
                                                                                                                                                                                                               
myMcp23017_rtos     *mcp_rtos;                                                                                                                                                                                        


dacRotary           *rotaryVoltage;



myMCP4725           *dacVoltage;
myMCP4725           *dacMaxCurrent;
simpler_INA219      *ina219;
WireBase            *DAC_I2C;


static void MainTask( void *a );

/*
 */

void initTft()
{
    if(tft)
    {
        delete tft;    
        tft=NULL;
    }
    // Deep reset of screen
    pinMode(TFT_RST,OUTPUT);
    digitalWrite(TFT_RST,LOW);
    
    digitalWrite(TFT_RST,HIGH);

    tft = new ILI9341(TFT_CS, TFT_DC,TFT_RST);
    tft->begin();  
    tft->fillScreen(ILI9341_BLACK);
    tft->setTextColor(ILI9341_WHITE,ILI9341_BLACK);  
    tft->setRotation(1); //3    
    tft->setFontFamily(&FreeSans9pt7b,&FreeSans18pt7b,&Targa56pt7b); //FreeSans24pt7b);
    
    tft->setFontSize(ILI9341::MediumFont);
    tft->fillScreen(ILI9341_RED);
}

void setup_vcc_sensor() 
{
    adc_reg_map *regs = ADC1->regs;
    regs->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and temp sensor
    regs->SMPR1 =  ADC_SMPR1_SMP17;  // sample rate for VREFINT ADC channel
}

#define BOOTUP(x,y,str) { tft->setCursor(x,y); tft->myDrawString(str);}

void mySetup() 
{
  Serial.println("Init"); 
  
  SPI.begin();
  SPI.setBitOrder(MSBFIRST); // Set the SPI bit order
  SPI.setDataMode(SPI_MODE0); //Set the  SPI data mode 0
  SPI.setClockDivider (SPI_CLOCK_DIV4); // Given for 10 Mhz...
  
  initTft();   
  Wire;
  TwoWire *tw=&Wire;;
  tw->begin();
  DAC_I2C=tw;
  tft->setFontSize(ILI9341::MediumFont);
  
  BOOTUP(10,10,"1-Setup DACs");
  
  dacVoltage=new myMCP4725(*DAC_I2C,VOLTAGE_ADC_I2C_ADR);
  dacVoltage->setVoltage(1100);  // 7v
  dacMaxCurrent =new myMCP4725(*DAC_I2C,MAXCURRENT_ADC_I2C_ADR);;
  dacMaxCurrent->setVoltage(0);  // 3*700/4096= 500 mA
  
  BOOTUP(10,40,"2-Setup INA");
        
  
  ina219=new    simpler_INA219(INA219_I2C_ADDR,100,&Wire);
  
  
  BOOTUP(10,70,"3-Setup IOs");

    // Reset MCP23017
  pinMode(MCP23017_RESET_PIN,OUTPUT);
  digitalWrite(MCP23017_RESET_PIN, LOW); 
  delay(100);
  digitalWrite(MCP23017_RESET_PIN, HIGH); 
 
  
  i2cMutex=new xMutex();
  mcp_rtos=new myMcp23017_rtos(MCP23017_INTERRUPT_PIN,i2cMutex); 
  
  rotaryVoltage=new   dacRotary(mcp_rtos,1000,25000,100,1000, 
                                1,0,
                                2,i2cMutex);
  
  mcp_rtos->start();    

  BOOTUP(10,100,"4-Starting FreeRTOS");
  
  // Ok let's go, switch to FreeRTOS
  xTaskCreate( MainTask, "MainTask", 500, NULL, 10, NULL );
  vTaskStartScheduler();

  
  
}
/**
 * 
 * @param 
 */
void MainTask( void *a )
{
    xDelay( 5);//yield
    tft->fillScreen(ILI9341_BLACK);
    tft->setTextColor(ILI9341_WHITE,ILI9341_BLACK);  
    char buffer[64];
    int value=0;
    while(1)
    {
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        
        // Grab INA219 Stuff
        i2cMutex->lock();
        int currentMA=ina219->getCurrent_mA();
        int voltageMV=(int)(1000.*ina219->getBusVoltage_V());
        i2cMutex->unlock();
        
        
        rotaryVoltage->run();
        value=rotaryVoltage->getValue();
        
        // Display
        tft->setCursor(10,20);
        sprintf(buffer,"Volt : %d mv\n",voltageMV);
        tft->myDrawString(buffer,280);

        tft->setCursor(10,50);
        sprintf(buffer,"Counter : %d \n",value);
        tft->myDrawString(buffer,280);

        xDelay( 150);
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        xDelay( 150);        
    }
}

void myLoop(void) 
{
    
}

#if 0
void myLoop(void) 
{
    char buffer[50];
#if 0
    static int lastConsignVoltage=-1;
    
    managePot(potAdc,dacVoltage,&lastConsignVoltage,20);
    static int lastConsignCurrent=-1;
    
    managePot(potCurrent,dacMaxCurrent,&lastConsignCurrent,20+50);
    
    float myLim=(float)lastConsignCurrent;
    myLim*=1.1;
    myLim+=60;
      
    
    int mil=floor(myLim);
    sprintf(buffer,"I:%04d mA",mil);    
#endif    
    int ma=ina219->getCurrent_mA();
    float mv=ina219->getBusVoltage_V();
    
    sprintf(buffer,"INA V=%2.2f",mv);
    tft->setCursor(20, 150);  
    tft->myDrawString(buffer,280);

    sprintf(buffer,"INA I=%d",ma);
    tft->setCursor(20,180);  
    tft->myDrawString(buffer,280);

    
#if 0
    tft->setCursor(20, 20+50+50);  
    tft->myDrawString(buffer,280);
#endif
    delay(100);

}
#endif
//-
