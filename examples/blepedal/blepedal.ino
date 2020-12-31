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
  //define your effect name
  name = "BLE PEDAL";

  //define the input mode (IM_LR or IM_LMIC)
  inputMode = IM_LR;

  //set up the controls
  control[0].name = "OUT LEVEL"; 
  control[0].mode = CM_POT;
  control[0].levelCount = 128; //(0-127)

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

//do repetitive task here (for debugging info only)
void loop() {

  //System info
  Serial.printf("\nSYSTEM INFO:\n");
  Serial.printf("Internal Total heap %d, internal Free Heap %d\n",ESP.getHeapSize(),ESP.getFreeHeap());
  Serial.printf("SPIRam Total heap %d, SPIRam Free Heap %d\n",ESP.getPsramSize(),ESP.getFreePsram());
  Serial.printf("ChipRevision %d, Cpu Freq %d, SDK Version %s\n",ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
  Serial.printf("Flash Size %d, Flash Speed %d\n",ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
  
  //Blackstomp application info
  Serial.printf("\nAPPLICATION INFO:\n");
  Serial.printf("Pedal Name: %s\n",myPedal.name.c_str());
  Serial.printf("Audio frame per second: %d fps\n",getAudioFps());
  Serial.printf("CPU ticks per frame period: %d\n",getTotalCpuTicks());
  Serial.printf("Used CPU ticks: %d\n",getUsedCpuTicks());
  Serial.printf("CPU Usage: %.2f %%\n", 100.0*getCpuUsage());
  
   vTaskDelay(1);
}
