/*!
 *  @file       stereochorus.ino
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

class stereoChorus:public effectModule
{
  private:
  float depth;
  float freq;
  float beatFrequency;
  float phaseDiff;
  fractionalDelay delay1;
  fractionalDelay delay2;
  oscillator lfo1;
  oscillator lfo2;
   
  public:
  void init();
  void deInit();
  void onControlChange(int controlIndex);
  void onButtonChange(int buttonIndex);
  void process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount);
};

////////////////////////////////////////////////////////////////////////
void stereoChorus::init()
{ 
  //select the appropriate device by uncommenting one of the following two lines:
  //setDeviceType(DT_ESP32_A1S_AC101);
  setDeviceType(DT_ESP32_A1S_ES8388);
  
  //default optimization for ES8388-version module  is 1/4 Vrms range
  //to optimize for the 1 Vrms range (more noisy), uncomment the following line:
  //optimizeConversion(0);
  
  //define your effect name
  name = "STEREO CHORUS";

  //define the input mode (IM_LR or IM_LMIC)
  inputMode = IM_LR;

  //set up the controls
  control[0].name = "Rate"; 
  control[0].mode = CM_POT;
  control[0].levelCount = 128; //(0-127)
  control[0].slowSpeed = true;

  control[2].name = "Depth"; 
  control[2].mode = CM_POT;
  control[2].levelCount = 128; //(0-127)
  control[2].slowSpeed = true;

  control[3].name = "Input Mode";
  control[3].mode = CM_SELECTOR;
  control[3].levelCount = 2; //0:mono, 1:stereo

  control[4].name = "F/P Diff"; 
  control[4].mode = CM_POT;
  control[4].levelCount = 128; //(0-127)
  control[4].slowSpeed = true;

  control[5].name = "Sync Mode";
  control[5].mode = CM_SELECTOR;
  control[5].levelCount = 2;

  //set up the buttons
  button[0].mode = BM_TOGGLE;
  
  delay1.init(3); //init for 3 ms delay
  delay2.init(3); //init for 3 ms delay
  freq=5;
  depth = 0.5;
  beatFrequency = 2.5;
  lfo1.setFrequency(freq);
  lfo2.setFrequency(freq+beatFrequency);
  
  //do resource allocation (if needed)..
  //..
} 

////////////////////////////////////////////////////////////////////////
void stereoChorus::deInit()
{
  //do resource deallocation (if needed)..
  //..
}

////////////////////////////////////////////////////////////////////////
void stereoChorus::onControlChange(int controlIndex)
{
  switch(controlIndex)
  {
    case 0: //rate
    {
      freq = 0.5+10*(float)control[0].value/127.0;
      lfo1.setFrequency(freq);
      lfo2.setFrequency(freq + beatFrequency);
      break;
    }
    case 2: //depth
    {
      depth = 1.49*(float)control[2].value/127.0;
      break;
    }
    case 3: //input mode
    {
      if(button[0].value == 0) //effect is bypassed
      {
      }
      break;
    }
    case 4: //phase or frequency difference
    {
      beatFrequency = 5*(float)control[4].value/127.0;
      phaseDiff = (float) control[4].value;
      lfo2.setFrequency(freq + beatFrequency);
      break;
    }
  }
}

void stereoChorus::onButtonChange(int buttonIndex)
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
void stereoChorus::process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount)
{   
  for(int i=0;i<sampleCount;i++)
  {
    delay1.write(inLeft[i]);
    delay2.write(inRight[i]); //write anyway, no matter it's sereo or mono input
      
    lfo1.update();
    lfo2.update();
    float dt1 = (1 + lfo1.getOutput())*depth;
    float dt2;
    if(control[5].value==0) //asynchronous
      dt2 = (1 + lfo2.getOutput())*depth;
    else  //synchronous
      dt2 = (1 + lfo1.getOutput((float)control[4].value))*depth;

    outLeft[i]=0.7*inLeft[i] + 0.7*delay1.read(dt1);
    if(control[3].value) //if stereo input
      outRight[i] = 0.7*inRight[i] + 0.7*delay2.read(dt2);
    else //if mono
      outRight[i]=0.7*inLeft[i] + 0.7*delay1.read(dt2);
  }
}

//declare an instance of your effect module
stereoChorus  myPedal;

//setup the effect modules by calling blackstompSetup() inside arduino core's setup()
void setup() {
  //SETTING UP THE EFFECT MODULE
  blackstompSetup(&myPedal);
}

//let the main loop empty to dedicate the core 1 for the main audio task
void loop() {
 
}
