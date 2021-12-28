/*!
 *  @file       midipedal.ino
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

/*This example sketch require the MIDI Library <MIDI.h> to be included,
 * so make sure it is installed on your IDE. If not then go to the library manager 
 * to install it (MIDI Library by Lathoub).
 */
#include <MIDI.h>
#include "blackstomp.h"

MIDI_CREATE_DEFAULT_INSTANCE();
class midiPedal:public effectModule
{  
  public:
  float gain;
  void init();
  void deInit();
  void onControlChange(int controlIndex);
  void onButtonChange(int buttonIndex);
  void process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount);
};

////////////////////////////////////////////////////////////////////////
void midiPedal::init()
{ 
  //select the appropriate device by uncommenting one of the following two lines:
  //setDeviceType(DT_ESP32_A1S_AC101);
  setDeviceType(DT_ESP32_A1S_ES8388);
  
  //default optimization for ES8388-version module  is 1/4 Vrms range
  //to optimize for the 1 Vrms range (more noisy), uncomment the following line:
  //optimizeConversion(0);
  
  //define your effect name
  name = "MIDI CONTROLLER";

  //define the input mode (IM_LR or IM_LMIC)
  inputMode = IM_LR;

  /* PART OF MIDI CONTROL FUNCTION (0-11)
 * 0 Bank Select 0-127 MSB
 * 1 Modulation Wheel or Lever 0-127 MSB
 * 2 Breath Controller 0-127 MSB
 * 3 Undefined 0-127 MSB
 * 4 Foot Controller 0-127 MSB
 * 5 Portamento Time 0-127 MSB
 * 6 Data Entry MSB  0-127 MSB
 * 7 Channel Volume (formerly Main Volume) 0-127 MSB
 * 8 Balance 0-127 MSB
 * 9 Undefined 0-127 MSB
 * 10 Pan 0-127 MSB
 * 11 Expression Controller
 */
  control[0].name = "Control Function";
  control[0].mode = CM_POT;
  control[0].levelCount = 12; //(0-11)
  
  control[1].name = "Out Level"; 
  control[1].mode = CM_POT;
  control[1].levelCount = 128; //(0-127)
  control[1].slowSpeed = true;
  
  control[2].name = "Channel";
  control[2].mode = CM_POT;
  control[2].levelCount = 16; //(0-15)

  //setup the buttons
  //main button
  button[0].mode = BM_TOGGLE;
  
  //do variable intitialization
  gain = 0;
  
  //do resource allocation (if needed)..
  //..
} 

////////////////////////////////////////////////////////////////////////
void midiPedal::deInit()
{
  //do resource deallocation (if needed)..
  //..
}

////////////////////////////////////////////////////////////////////////
void midiPedal::onControlChange(int controlIndex)
{
  switch(controlIndex)
  {
    case 0: //control[0]
    {
      auxLed->blink(10,10,1,1,0);
      break;
    }
    case 1:
    {
      auxLed->blink(10,10,1,1,0);
      gain = (float)control[1].value/127.0;
      MIDI.sendControlChange(control[0].value,control[1].value,control[2].value+1);
      break;
    }
    case 2:
    {
      auxLed->blink(10,10,1,1,0);
      break;
    }
  }
}

void midiPedal::onButtonChange(int buttonIndex)
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

////////////////////////////////////////////////////////////////////////
void midiPedal::process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount)
{   
  for(int i=0;i<sampleCount;i++)
  {
    outLeft[i]=inLeft[i] * gain;
    outRight[i]=inRight[i] * gain;
  }
}

//declare an instance of your effect module
midiPedal myPedal;

void handleControlChange(byte channel, byte controlNumber, byte controlValue)
{
  if(channel==myPedal.control[2].value+1)
  {
    if(controlNumber==myPedal.control[0].value)
    {
      myPedal.auxLed->blink(10,10,1,1,0);
      myPedal.gain = (float)controlValue/127.0;
    }
  }
}


//create MIDI task to handle the MIDI input stream on core 0,
//so we can  dedicate core 1 for signal processing 
void midiTask(void* arg)
{
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleControlChange(handleControlChange);
  while(true)
  {
    MIDI.read();
    vTaskDelay(1);
  }
}

//setup the effect modules by calling blackstompSetup() inside arduino core's setup()
void setup() {
  //SETTING UP THE EFFECT MODULE
  blackstompSetup(&myPedal);
  
  //start the MIDI input processing task on core 0
  xTaskCreatePinnedToCore(midiTask, "midiTask",4096,NULL,9,NULL,0);
}

//let the main loop empty to dedicate the core 1 for the main audio task
void loop() {

}
