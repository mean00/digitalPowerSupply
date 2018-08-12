/**
 */
#pragma once
#include "adc.h"
class myAdc
{
public:
              myAdc(int pin,float scaler);
        float getValue();
        int   getRawValue();
  
protected:
        adc_dev *_device;
        int     _channel;
        float   _scaler;
};