/*!
 *  @file       distortion.ino
 *  Project     Blackstomp Arduino Library
 *  @brief      Blackstomp Library for the Arduino
 *  @author     Hasan Murod
 *  @date       09/12/2022
 *  @license    MIT - Copyright (c) 2022 Hasan Murod
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

class distortion:public effectModule
{
  private:
    waveShaper dist;
    waveShaper dist2;
    rcHiPass decoupler;
    rcHiPass decoupler2;
    simpleTone tonecontrol;
    noiseGate gate;
    float inGain;
    float outGain;
  public:
  void init();
  void deInit();
  void onControlChange(int controlIndex);
  void onButtonChange(int buttonIndex);
  void process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount);
};

////////////////////////////////////////////////////////////////////////
void distortion::init()
{ 
  //select the appropriate device by uncommenting one of the following two lines:
  //setDeviceType(DT_ESP32_A1S_AC101);
  setDeviceType(DT_ESP32_A1S_ES8388);
  
  //default optimization for ES8388-version module  is 1/4 Vrms range
  //to optimize for the 1 Vrms range (more noisy), uncomment the following line:
  //optimizeConversion(0);
  
  //define your effect name
  name = "DISTORTION";

  //define the input mode (IM_LR or IM_LMIC)
  inputMode = IM_LR;

  //set up the controls
  control[0].name = "Out Level"; 
  control[0].mode = CM_POT;
  control[0].levelCount = 128; //(0-127)
  control[0].slowSpeed = true;

  control[1].name = "Gain"; 
  control[1].mode = CM_POT;
  control[1].levelCount = 128; //(0-127)
  control[1].slowSpeed = true;

  control[2].name = "Tone";
  control[2].mode = CM_POT;
  control[2].levelCount = 128; //(0-127)
  control[2].slowSpeed = true;

  control[4].name = "Noise Gate";
  control[4].mode = CM_POT;
  control[4].levelCount = 128; //(0-127)
  control[4].slowSpeed = true;

  inGain=1;
  outGain=1;

  //DISTORTION
  //You can customize the distortion element dist and dist2 by modifying the transferFunctionTable member (256 array elements)
  /*
   * dist.transferFunctionTable[0]=-1;
   * dist.transferFunctionTable[1]=-0.99;
   * ..
   * ..
   * dist.transferFunctionTable[255]  = 1;
   * 
   * dist2.transferFunctionTable[0]=-1;
   * ..
   * dist2.transferFunctionTable[255]=1;
   * */

   //TONE CONTROL
   //to change from the default value, you can uncomment the following 2 lines to edit the high pass and the low passs filters of the tone control section
   //tonecontrol.hiPass.setTimeConstant(0.0001078); //default = 0.0001078 (emulate big muff R22k and C4.9nF)
   //tonecontrol.loPass.setTimeConstant(0.00039); //default = 0.00039 (emulate big muff R39k and C10nF)

  //set up the buttons
  button[0].mode = BM_TOGGLE;
  
  //do resource allocation (if needed)..
  //..
} 

////////////////////////////////////////////////////////////////////////
void distortion::deInit()
{
  //do resource deallocation (if needed)..
  //..
}

////////////////////////////////////////////////////////////////////////
void distortion::onControlChange(int controlIndex)
{
  switch(controlIndex)
  {
    case 0: //out level
    {
      outGain = powf((float)control[0].value/127.0f,2);
      break;
    }
    case 1: //gain
    {
      inGain = 50.0f* (float)control[1].value/127.0f;
      break;
    }
    case 2: //tone
    {
      tonecontrol.setTone((float)control[2].value/127.0f);
      break;
    }
    case 4: //noise gate
    {
      float val = (float)control[4].value/127.0f;
      gate.setThreshold(val);
      break;
    }
  }
}

void distortion::onButtonChange(int buttonIndex)
{
   switch(buttonIndex)
   {
    case 0:
    {
      if(button[0].value)
      {
        analogBypass(false);
        mainLed->turnOn();
      }
      else
      {
        analogBypass(true);
        mainLed->turnOff();
      }
      break;
    }
   }
};

////////////////////////////////////////////////////////////////////////
void distortion::process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount)
{   
  for(int i=0;i<sampleCount;i++)
  {
    float temp = inLeft[i]; 
    temp = gate.process(temp);
    temp = inGain * temp;
    temp = dist.process(temp);
    temp = decoupler.process(temp);
    temp = dist2.process(temp);
    temp = decoupler2.process(temp);
    temp = tonecontrol.process(temp);
    temp = outGain * temp;
    outLeft[i] = temp;
  }
}

//declare an instance of your effect module
distortion  myPedal;

//setup the effect modules by calling blackstompSetup() inside arduino core's setup()
void setup() {
  //SETTING UP THE EFFECT MODULE
  blackstompSetup(&myPedal);

  runSystemMonitor();
}

//let the main loop empty to dedicate the core 1 for the main audio task
void loop() {
 
}
