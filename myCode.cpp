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
#include "myAdc/myAdc.h"


#define DCDC_ENABLE_PIN         PB4

// ILI9341 is using HW SPI + those pins
// HW SPI= pin                  PA5/PA6/PA7
#define TFT_DC                  PB0
#define TFT_RST                 PB1 
#define TFT_CS                  PB10
//
#define MCP23017_INTERRUPT_PIN  PC15
#define PERIPHERALS_RESET_PIN   PA0
// I2C MAPPING
// HW I2C = pins PB6/PB7

#define VOLTAGE_ADC_I2C_ADR     0x61
#define MAXCURRENT_ADC_I2C_ADR  0x60
#define MCP23017_I2C_ADDR       0x00 // Actual =0x20+0
#define INA219_I2C_ADDR         0x40


// ADC IN : A2, A3

// Our globals

xMutex              *i2cMutex; // to protect i2c from concurrent accesses

ILI9341             *tft=NULL;
                                                                                                                                                                                                               
myMcp23017_rtos     *mcp_rtos;                                                                                                                                                                                        

dacRotary           *rotaryVoltage;
dacRotary           *rotaryAmp;
myMCP4725           *dacVoltage;
myMCP4725           *dacMaxCurrent;
simpler_INA219      *ina219;
WireBase            *DAC_I2C;


static void MainTask( void *a );

#define HAS_TFT 1

myAdc *adcVoltage,*adcAmps;
/*
 */
#if HAS_TFT
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
    tft->setRotation(1); //1--3    
    tft->setFontFamily(&FreeSans9pt7b,&FreeSans18pt7b,&Targa56pt7b); //FreeSans24pt7b);
    
    tft->setFontSize(ILI9341::MediumFont);
    tft->fillScreen(ILI9341_RED);
}
#endif
/**
 * 
 */
void setup_vcc_sensor() 
{
    adc_reg_map *regs = ADC1->regs;
    regs->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and temp sensor
    regs->SMPR1 =  ADC_SMPR1_SMP17;  // sample rate for VREFINT ADC channel
}
#if HAS_TFT
#define BOOTUP(x,y,str) { tft->setCursor(x,y); tft->myDrawString(str);}
#else
#define BOOTUP(...) {}
#endif
/**
 * 
 */
void mySetup() 
{
  Serial.println("Init"); 
  
  pinMode(DCDC_ENABLE_PIN, OUTPUT); // disable DCDC
  digitalWrite(DCDC_ENABLE_PIN, LOW); 
  
  pinMode(PERIPHERALS_RESET_PIN,OUTPUT);
  digitalWrite(PERIPHERALS_RESET_PIN, LOW); 
    // Reset MCP23017 & friends
  delay(10);
  digitalWrite(PERIPHERALS_RESET_PIN, HIGH); 
  delay(10);

  
  
  SPI.begin();
  SPI.setBitOrder(MSBFIRST); // Set the SPI bit order
  SPI.setDataMode(SPI_MODE0); //Set the  SPI data mode 0
  SPI.setClockDivider (SPI_CLOCK_DIV32); // Given for 10 Mhz... SPI_CLOCK_DIV4
#if HAS_TFT  
  initTft();   
#endif
  Wire;
  TwoWire *tw=&Wire;;
  tw->begin();
  DAC_I2C=tw;
#if HAS_TFT
  tft->setFontSize(ILI9341::MediumFont);
#endif

  BOOTUP(10,10,"1-Setup DACs");
  
  dacVoltage=new myMCP4725(*DAC_I2C,VOLTAGE_ADC_I2C_ADR);
  dacVoltage->setVoltage(1100);  // 7v
  dacMaxCurrent =new myMCP4725(*DAC_I2C,MAXCURRENT_ADC_I2C_ADR);;
  dacMaxCurrent->setVoltage(0);  // 3*700/4096= 500 mA
  
  BOOTUP(10,40,"2-Setup INA");
        
#if 0    
  ina219=new    simpler_INA219(INA219_I2C_ADDR,100,&Wire);  
#endif  
  
  BOOTUP(10,70,"3-Setup IOs");

  
  i2cMutex=new xMutex();
  mcp_rtos=new myMcp23017_rtos(MCP23017_INTERRUPT_PIN,i2cMutex); 
  
  rotaryVoltage=new   dacRotary(mcp_rtos,4000,28000,100,1000, 
                                0,1, // up down
                                2, // button
                                i2cMutex);
  
  rotaryAmp = new  dacRotary(mcp_rtos,
                                0,4096, // min max
                                10,100,  // steps
                                3,4, // up down
                                5, // button
                                i2cMutex);
  
  mcp_rtos->start();    
  BOOTUP(10,100,"4-Starting ADCs");  
  adcVoltage=new myAdc(PA2,11.1);
  adcAmps=new myAdc(PA3,1.0);

  
  BOOTUP(10,130,"5-Starting FreeRTOS");
  
  // Ok let's go, switch to FreeRTOS
  xTaskCreate( MainTask, "MainTask", 500, NULL, 10, NULL );
  vTaskStartScheduler();  
}
/**
 * 
 * @param v
 * @return 
 */
int mV2command(int v)
{
    if(v<4000) v=4000;
    if(v>28000) v=28000;
    float f=v;
    
    f-=1475;    
    f/=9.75;
    
    return (int)(f+0.45);
    
}

/**
 * 
 * @param v
 * @return 
 */
int amp2command(int ma)
{
    if(ma<0) ma=0;
    if(ma>3500) ma=3500;
    float f=ma;
    
    f-=100;    
    f/=1.1;
    
    return (int)(f+0.45);
    
}


/**
 * 
 * @param 
 */
void MainTask( void *a )
{
    xDelay( 5);//yield
#if HAS_TFT    
    tft->fillScreen(ILI9341_BLACK);
    tft->setTextColor(ILI9341_WHITE,ILI9341_BLACK);  
#endif
    char buffer[64];
    
    
    int value=5000; // 5v
    rotaryVoltage->setValue(value);
    dacVoltage->setVoltage(mV2command(value));
    int ping=0;
    int maxCurrent=2048;
    // unlock DC/DC
    delay(100);
    digitalWrite(DCDC_ENABLE_PIN, HIGH); 
    delay(100);
    while(1)
    {
        
        
        // Grab INA219 Stuff
#if 0
        i2cMutex->lock();
        int currentMA=ina219->getCurrent_mA();
        int voltageMV=(int)(1000.*ina219->getBusVoltage_V());
        i2cMutex->unlock();
#endif   
        
#define STEP 40     
        
        // Voltage
        
        rotaryVoltage->run();
        int nvalue=rotaryVoltage->getValue();
        
        {
            float v= adcVoltage->getValue();
            tft->setCursor(10,10);
            sprintf(buffer,"ADCV:%02.2f v",v);
            tft->myDrawString(buffer,280);

        }
        {
            float a=adcAmps->getRawValue();
            tft->setCursor(10,1+STEP*3);
            sprintf(buffer,"AA:%02.2f",a);
            tft->myDrawString(buffer,280);
        
        }
        if(value!=nvalue)
        {
            
            
  
            
            int cmd=mV2command(nvalue); // 0..30000
            dacVoltage->setVoltage(cmd);
            value=nvalue;
            
            tft->setCursor(10,10+STEP);
            sprintf(buffer,"V(mv):%d:%d",value,cmd);
            tft->myDrawString(buffer,280);

        }
        
        // current
        
        rotaryAmp->run();
        int current=rotaryAmp->getValue();
        if(current!=maxCurrent)
        {
            maxCurrent=current;
            
            int currentCommand=amp2command(maxCurrent);
            
            dacMaxCurrent->setVoltage(currentCommand);
        
            tft->setCursor(10,10+STEP*4);
            sprintf(buffer,"A :%d:%d \n",current,currentCommand);
            tft->myDrawString(buffer,280);
            
        }
#if HAS_TFT
        tft->setCursor(200,200);
        sprintf(buffer," %d ",ping);
        tft->myDrawString(buffer,200);
        ping++;
#endif        

        xDelay( 200);
        
    }
}
/**
 * 
 */
void myLoop(void) 
{
    
}


//-
