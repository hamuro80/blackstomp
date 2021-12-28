/*!
 *  @file       blepedal.ino
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

class blePedal:public effectModule
{
  private:
  float gain;
   
  public:
  void init();
  void deInit();
  void onControlChange(int controlIndex);
  void onButtonChange(int buttonIndex);
  void process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount);
  void onBleTerminalRequest(const char* request, char* response);
};

////////////////////////////////////////////////////////////////////////
void blePedal::init()
{ 
  //select the appropriate device by uncommenting one of the following two lines:
  //setDeviceType(DT_ESP32_A1S_AC101);
  setDeviceType(DT_ESP32_A1S_ES8388);
  
  //default optimization for ES8388-version module  is 1/4 Vrms range
  //to optimize for the 1 Vrms range (more noisy), uncomment the following line:
  //optimizeConversion(0);
  
  //define your effect name
  name = "BLE PEDAL";

  //define the input mode (IM_LR or IM_LMIC)
  inputMode = IM_LR;

  //set up the controls
  control[0].name = "OUT LEVEL"; 
  control[0].mode = CM_POT;
  control[0].levelCount = 128; //(0-127)
  control[0].slowSpeed = true;

  //set up the buttons
  button[0].mode = BM_TOGGLE;
  
  //do variable intitialization
  gain = 0;
  
  //do resource allocation (if needed)..
  //..
} 

////////////////////////////////////////////////////////////////////////
void blePedal::deInit()
{
  //do resource deallocation (if needed)..
  //..
}

////////////////////////////////////////////////////////////////////////
void blePedal::onControlChange(int controlIndex)
{
  switch(controlIndex)
  {
    case 0: //control[0]
    {
      gain = (float)control[0].value/127.0;
      break;
    }
  }
}

void blePedal::onButtonChange(int buttonIndex)
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
}

//BLE terminal handler
void blePedal::onBleTerminalRequest(const char* request, char* response)
{
  strcpy(response,"TRequest has been received and processed");
}

////////////////////////////////////////////////////////////////////////
void blePedal::process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount)
{   
  for(int i=0;i<sampleCount;i++)
  {
    outLeft[i]=inLeft[i] * gain;
    outRight[i]=inRight[i] * gain;
  }
}

//declare an instance of your effect module
blePedal  myPedal;

//setup the effect modules by calling blackstompSetup() inside arduino core's setup()
void setup() {
  //SETTING UP THE EFFECT MODULE
  blackstompSetup(&myPedal);
  enableBleTerminal();
}

//let the main loop empty to dedicate the core 1 for the main audio task
void loop() {

}
