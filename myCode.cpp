/***************************************************
 Digitally controlled power supply
 *  * GPL v2
 * (c) mean 2018 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "SPI.h"
#include "ILI9341_extended.h"

#include "Fonts/FreeSans24pt7b.h"
#include "Fonts/FreeSans18pt7b.h"
#include "Fonts/FreeSans9pt7b.h"

// ILI9341 is using HW SPI + those pins
#define TFT_DC          PB0
#define TFT_RST         PB1
#define TFT_CS          PB10

// Our globals
ILI9341              *tft=NULL;
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
    tft->setRotation(3);
    tft->setFontFamily(&FreeSans9pt7b,&FreeSans18pt7b,&FreeSans24pt7b);
    tft->setFontSize(ILI9341::MediumFont);
    tft->fillScreen(ILI9341_RED);
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
    "aaaaa",
    "ZZZZZZZ"
};
void myLoop(void) 
{
    static bool draw=true;
    static int offset=3;
    if(draw)
    {
#if 0
        tft->setFontSize(ILI9341::SmallFont);
        for(int i=0;i<5;i++)
        {
            int y=(i+offset)%5;        
            tft->setCursor(2, 38+20*i);       
            tft->myDrawString(text[y]);
        }
#else
        tft->setFontSize(ILI9341::MediumFont);
         tft->setCursor(2, 38);   
         int y=offset%5;
          tft->myDrawString(text2[y],100);
#endif        
        offset++;        
        draw=false;        
    }

}
//-