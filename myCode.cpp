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


// ILI9341 is using HW SPI + those pins
#define TFT_DC          PA8 // OK
#define TFT_RST         PA9 // OK
#define TFT_CS          PA10 // PB10
#define PIN_POT_VOLTAGE PB0
#define PIN_VOLTOUT_VOLTAGE     PB1
#define VOLTAGE_ADC_I2C_ADR 0x61

// Our globals
ILI9341              *tft=NULL;
myAdc                *potAdc;
myAdc                *voltageAdc;
float cal;

adc_dev *voltageADCDevice;
int     voltageADCChannel;
myMCP4725 *dacVoltage;

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
  
 
  
  Serial.println("TFT"); 
  initTft();   
  
#if 1  
  DAC_I2C=new SoftWire(PB8/* SCL */,PB9 /* SDA*/);
  DAC_I2C->begin();
#else
  pinMode(PB8,OUTPUT);
  pinMode(PB9,OUTPUT);
  TwoWire *tw=new TwoWire(1);
  tw->setClock(100000);
  tw->begin();
  DAC_I2C=tw;
#endif  
  
  
  potAdc=new myAdc(PIN_POT_VOLTAGE,1.);
  voltageAdc=new myAdc(PIN_VOLTOUT_VOLTAGE,11.);
  
   
  
  dacVoltage=new myMCP4725(*DAC_I2C,VOLTAGE_ADC_I2C_ADR);
  
}
/**
 */

void myLoop(void) 
{

    char buffer[20];
    
    int consign=potAdc->getRawValue();
    dacVoltage->setVoltage(consign);
    float v;

#if 0    
    v=potAdc->getValue();
    sprintf(buffer,"%02.3f",v);
    tft->setFontSize(ILI9341::BigFont);
    tft->setCursor(20, 20);  
    tft->myDrawString(buffer,280);
#else
    sprintf(buffer,"%03d",consign);
    tft->setFontSize(ILI9341::BigFont);
    tft->setCursor(20, 20);  
    tft->myDrawString(buffer,280);
    
#endif
    
  
    
    
    v=voltageAdc->getValue();
    sprintf(buffer,"%02.3f",v);
    tft->setFontSize(ILI9341::BigFont);
    tft->setCursor(20, 120+20);  
    tft->myDrawString(buffer,280);
    
    
    
    delay(100);

}
//-