/***************************************************
 Digitally controlled power supply
 *  * GPL v2
 * (c) mean 2018 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "SPI.h"
#include "ILI9341_extended.h"
#include "adc.h"
//#include "Fonts/FreeSans24pt7b.h"
#include "Fonts/digitLcd56.h"
#include "Fonts/FreeSans18pt7b.h"
#include "Fonts/FreeSans9pt7b.h"

// ILI9341 is using HW SPI + those pins
#define TFT_DC          PA8 // OK
#define TFT_RST         PA9 // OK
#define TFT_CS          PA10 // PB10
#define PIN_ADC_VOLTAGE PB0
// Our globals
ILI9341              *tft=NULL;
float cal;
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
    tft->setFontFamily(&FreeSans9pt7b,&FreeSans18pt7b,&DIGIT_LCD56pt7b); //FreeSans24pt7b);
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
  // Init PB0 as ADC in
  
    pinMode(PIN_ADC_VOLTAGE,INPUT_ANALOG);
    setup_vcc_sensor();
    
    cal = 1.  / (float)adc_read( ADC1, 17);
    cal=1200.*4.096*cal;

}

/**
 */
const char *text[5]={
    " This is First LINE",
    "ABCDEFGHIJKLM",
    "<           ->",
    "0123456789",
    "tHIS is Last lINE"
};
const char *text2[6]={
    "Z",
    "aa",
    "ZZZ",
    "9.20 V",
    "ZZZZZZZ"
};


void myLoop(void) 
{

    char buffer[20];
    float  millivolts ;
   
    
#define NB 8
    float     sum=0,v;
    for(int i=0;i<NB;i++)
    {
        v = (float)analogRead(PIN_ADC_VOLTAGE);
     
        sum+=v;
    }
    v=sum/NB;
    v=floor(((v*cal)/4096.)*1000.);
    v=(v)/1000.;
    sprintf(buffer,"%02.3f",v);
    tft->setFontSize(ILI9341::BigFont);
    tft->setCursor(2, 120);  
    tft->myDrawString(buffer,300);

}
//-