#include "math.h"
#include "adc.h"
#include "myAdc.h"
#include <Wire.h>

static float ad_voltage_cal=1;
static bool calibrated=false;

static void myAdc_calibrate();

/**
+ */
myAdc::myAdc(int pin, float scaler)
{
    if(!calibrated)
    {
        calibrated=true;
        myAdc_calibrate();
    }
    pinMode(pin,INPUT_ANALOG);
    _device=PIN_MAP[pin].adc_device;
    _channel=PIN_MAP[pin].adc_channel;
    _scaler=scaler;
    adc_set_sample_rate(_device,ADC_SMPR_239_5); // slow is fine :)
    adc_calibrate(_device); // called multiple time potentially, does not matter
}
/**
+ * 
+ * @return 
+ */
#define NB 4
float myAdc::getValue()
{
    float v,sum=0;
   for(int i=0;i<NB;i++)
    {
        
        //   v = (float)analogRead(PIN_ADC_VOLTAGE);
        v=(float) adc_read(_device,_channel);
       sum+=v;
    }
    v=sum/NB;
    v=floor(((v*ad_voltage_cal)/4095.)*_scaler*1000.);
    v=(v)/1000.;
    return v;
}

int   myAdc::getRawValue()
{
    int v,sum=0;
    for(int i=0;i<NB;i++)
    {
        
        //   v = (float)analogRead(PIN_ADC_VOLTAGE);
        v= adc_read(_device,_channel);
        sum+=v;
    }
    v=sum/NB;
    return v;
}
/**
+ * \fn probe for VCC used for scale
+ */
void myAdc_calibrate()
{
    adc_reg_map *regs = ADC1->regs;
    regs->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and temp sensor
    regs->SMPR1 =  ADC_SMPR1_SMP17;  // sample rate for VREFINT ADC channel
    ad_voltage_cal = 1.  / (float)adc_read( ADC1, 17);
    ad_voltage_cal=1200.*4.095*ad_voltage_cal;
}
