/**
 */
#pragma once
#include "mcp23017.h"
#include "MCP23017_rtos.h"

class dacRotary
{
public:
    dacRotary(myMcp23017_rtos *mcp,int xmin, int xmax, int stepLow, int stepHigh, int pinUp, int pinDown, int pinPush,xMutex *mutex)
    {
        _min=xmin;
        _max=xmax;
        _stepLow=stepLow;
        _stepHigh=stepHigh;
        _mutex=mutex;
        _mcp=mcp;
        _value=0;
        _low=false;
        // create ..
        pushButton=new myMcpButtonInput(_mcp->mcp(),pinPush); // A2
        rotary=new myMcpRotaryEncoder(_mcp->mcp(),pinUp,pinDown);              
    }
    
    void run()
    {
      int mul=_stepHigh;
      if(_low)
        mul=_stepLow;
      _value=_value+mul*rotary->count();
      if(_value<_min) _value=_min;
      if(_value>_max) _value=_max;
      if(pushButton->changed())
        _low=pushButton->state();
      
    }
    int getValue()
    {
      return _value;
    }
    void setValue(int v)
    {
      _value=v;
    }

  
protected:
        bool  _low;
        int _value;
        int _min;
        int _max;
        int _stepLow;
        int _stepHigh;
        myMcp23017_rtos *_mcp;
        xMutex *_mutex;
        myMcpButtonInput    *pushButton;                                                                                                                                                                                     
        myMcpRotaryEncoder  *rotary;     
        
  
};