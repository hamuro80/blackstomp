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
  name = "MIC MIXER";
  inputMode = IM_LMIC;

  control[0].name = "Mic Gain";
  control[0].mode = CM_POT;
  control[0].levelCount = 8;  //0-7: 0dB, 30dB, 33dB, 36dB, 39dB, 42dB, 45dB, 48dB
  control[0].value = 0;

  control[2].name = "Mic Level"; 
  control[2].mode = CM_POT;
  control[2].levelCount = 128; 
  control[2].value = 64;
  
  //setup the buttons
  //main button
  button[0].mode = BM_TOGGLE;

  //other initialization
  micLevel = 1;
  setMicGain(0);
} 

////////////////////////////////////////////////////////////////////////
void micMixer::deInit()
{
  //do nothing
}

////////////////////////////////////////////////////////////////////////
void micMixer::onControlChange(int controlIndex)
{
  Serial.printf("%s : %d\r\n",control[controlIndex].name.c_str(),control[controlIndex].value);
  switch(controlIndex)
  {
    case 0: //mic gain
    {
      setMicGain(control[0].value);
      break;
    }
    case 2: //mic level
    {
      micLevel = (float)control[2].value/127.0;
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
void setup() {
 //Serial communication
  Serial.begin(115200);

  //SETTING UP THE EFFECT MODULE
  blackstompSetup(&myPedal);
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
  vTaskDelay(1000);
}
