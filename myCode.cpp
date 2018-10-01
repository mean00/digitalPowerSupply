/***************************************************
 Digitally controlled power supply
 *  * GPL v2
 * (c) mean 2018 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "SPI.h"
#include "ILI9341_extended.h"
#include "adc.h"
#include "myAdc.h"
#include "Fonts/digitLcd56.h"
#include "Fonts/Targ56.h"
#include "Fonts/FreeSans18pt7b.h"
#include "Fonts/FreeSans9pt7b.h"
#include "myMCP4725.h"
#include "SoftWire.h"
#include "Wire.h"
#include "simpler_INA219.h" // Add dummy comment to make arduino cmake not consider this line

// ILI9341 is using HW SPI + those pins
#define TFT_DC                  PA8 // OK
#define TFT_RST                 PA9 // OK
#define TFT_CS                  PA10 // PB10
#define PIN_POT_VOLTAGE         PB0
#define PIN_VOLTOUT_VOLTAGE     PB1
#define VOLTAGE_ADC_I2C_ADR    0x61
#define MAXCURRENT_ADC_I2C_ADR 0x60

// Our globals
ILI9341              *tft=NULL;
myAdc                *potAdc;
myAdc                *potCurrent;




myMCP4725 *dacVoltage;
myMCP4725 *dacMaxCurrent;
simpler_INA219 *ina219;
WireBase *DAC_I2C;

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
    //tft->setFontFamily(&FreeSans9pt7b,&FreeSans18pt7b,&DIGIT_LCD56pt7b); //FreeSans24pt7b);
    tft->setFontFamily(&FreeSans9pt7b,&FreeSans18pt7b,&Targa56pt7b); //FreeSans24pt7b);
    
    tft->setFontSize(ILI9341::MediumFont);
    tft->fillScreen(ILI9341_RED);
}

void setup_vcc_sensor() {
    adc_reg_map *regs = ADC1->regs;
    regs->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and temp sensor
    regs->SMPR1 =  ADC_SMPR1_SMP17;  // sample rate for VREFINT ADC channel
}

void mySetup() 
{
  Serial.println("Init"); 
  
  SPI.begin();
  SPI.setBitOrder(MSBFIRST); // Set the SPI bit order
  SPI.setDataMode(SPI_MODE0); //Set the  SPI data mode 0
  SPI.setClockDivider (SPI_CLOCK_DIV4); // Given for 10 Mhz...
  
  initTft();   
  Wire;
#if 0
  DAC_I2C=new SoftWire(PB6/* SCL */,PB7 /* SDA*/);
  DAC_I2C->begin();
#else 
  TwoWire *tw=&Wire;;
  tw->begin();
  DAC_I2C=tw;
#endif  
  
  
  potAdc=new myAdc(PIN_POT_VOLTAGE,1.);
  potCurrent=new myAdc(PIN_VOLTOUT_VOLTAGE,1.);
  
  dacVoltage=new myMCP4725(*DAC_I2C,VOLTAGE_ADC_I2C_ADR);
  dacVoltage->setVoltage(1100);  // 7v
  dacMaxCurrent =new myMCP4725(*DAC_I2C,MAXCURRENT_ADC_I2C_ADR);;
  dacMaxCurrent->setVoltage(0);  // 3*700/4096= 500 mA
  
  tft->setFontSize(ILI9341::MediumFont);
  
  ina219=new    simpler_INA219(0x40,100,&Wire);
  
}
/**
 */


void managePot(myAdc *adc,myMCP4725 *dac, int *lastValue , int pos)
{
    char buffer[20];
    int consign=adc->getRawValue(); //0...4096
    consign=(consign+10)/20;
    consign*=20;
    // Round up to the closest 20
    if(abs(consign-*lastValue)>20 || *lastValue==-1)
    {
        *lastValue=consign;
        dac->setVoltage(consign);
        sprintf(buffer,"%03d",consign);
#if 0        
        tft->setCursor(20, pos);  
        tft->myDrawString(buffer,280);
#endif
    }
}

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
//-
