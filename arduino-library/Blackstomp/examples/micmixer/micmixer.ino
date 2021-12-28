/*!
 *  @file       micmixer.ino
 *  Project     Blackstomp Arduino Library
 *  @brief      Blackstomp Library for the Arduino
 *  @author     Hasan Murod
 *  @date       19/11/2020
 *  @license    MIT - Copyright (c) 2020 Hasan Murod
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "blackstomp.h"

class micMixer:public effectModule
{
  private:
  float micLevel;
  public:
  void init();
  void deInit();
  void onControlChange(int controlIndex);
  void onButtonChange(int buttonIndex);
  void process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount);
};

////////////////////////////////////////////////////////////////////////
void micMixer::init()
{
  //select the appropriate device by uncommenting one of the following two lines:
  //setDeviceType(DT_ESP32_A1S_AC101);
  setDeviceType(DT_ESP32_A1S_ES8388);
  
  //default optimization for ES8388-version module  is 1/4 Vrms range
  //to optimize for the 1 Vrms range (more noisy), uncomment the following line:
  //optimizeConversion(0);
  
  name = "MIC MIXER";
  inputMode = IM_LMIC;

  control[0].name = "Mic Gain";
  control[0].mode = CM_POT;
  control[0].levelCount = 8;  //0-7: 0dB, 30dB, 33dB, 36dB, 39dB, 42dB, 45dB, 48dB
  control[0].value = 0;

  control[1].name = "Noise Gate";
  control[1].mode = CM_POT;
  control[1].levelCount = 33;  //0-32
  control[1].value = 0;

  control[2].name = "Mic Level"; 
  control[2].mode = CM_POT;
  control[2].levelCount = 128; 
  control[2].value = 64;
  control[2].slowSpeed = true;
  
  //setup the buttons
  //main button
  button[0].mode = BM_TOGGLE;

  //other initialization
  micLevel = 1;
} 

////////////////////////////////////////////////////////////////////////
void micMixer::deInit()
{
  //do nothing
}

////////////////////////////////////////////////////////////////////////
void micMixer::onControlChange(int controlIndex)
{
  switch(controlIndex)
  {
    case 0: //mic gain
    {
      setMicGain(control[0].value);
      break;
    }
    case 1: //noise gate
    {
      setMicNoiseGate(control[1].value);
      break;
    }
    case 2: //mic level
    {
      micLevel = 4*(float)control[2].value/127.0;
      break;
    }
  }
}

void micMixer::onButtonChange(int buttonIndex)
{
   switch(buttonIndex)
   {
    case 0: //main button
    {
      if(button[0].value) //if effect is activated
      {
        mainLed->turnOn();
      }
      else //if effect is bypassed
      {
        mainLed->turnOff();
      }
      break;
    }
   }
};
 
////////////////////////////////////////////////////////////////////////
void micMixer::process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount)
{   
  for(int i=0;i<sampleCount;i++)
  {
    if(button[0].value) //effect is activated
    {
      outLeft[i] = inLeft[i]+micLevel*inRight[i];
      outRight[i] = outLeft[i];
    }
    else //effect is deactivated, software-bypassed
    {
      outLeft[i] = inLeft[i];
      outRight[i] = inLeft[i];
    }
  }
}

//declare an instance of your effect module
micMixer  myPedal;

//setup the effect modules by calling blackstompSetup() inside arduino core's setup()
void setup() 
{;
  //SETTING UP THE EFFECT MODULE
  blackstompSetup(&myPedal);
}

//let the main loop empty to dedicate the core 1 for the main audio task
void loop() 
{
 
}
