/*!
 *  @file       taptempodelay.ino
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

class taptempoDelay:public effectModule
{
  private:
  float* delayBufferL;
  float* delayBufferR;
  int readIndex;
  int writeIndex;
  int indexShift;
  int filteredIndexShift;
  void setTime(int ms);
  void updateIndex();
  float dryGain;
  float wetGain;
  float feedbackGain;
  
  public:
  void init();
  void deInit();
  void onControlChange(int controlIndex);
  void onButtonChange(int buttonIndex);
  void process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount);
};

////////////////////////////////////////////////////////////////////////
void taptempoDelay::init()
{
  Serial.printf("Initializing the effect module..\n");
  
  name = "TAP-TEMPO DELAY";
  inputMode = IM_LR;

  control[0].name = "Time";
  control[0].mode = CM_POT;
  control[0].levelCount = 200;  //0 = 10 ms, 199 = 2000 ms
  control[0].value = 49; //500 ms

  control[2].name = "Repeat";  //feedback gain
  control[2].mode = CM_POT;
  control[2].levelCount = 128; 
  control[2].value = 64;
  
  //set up the controls
  control[3].name = "Input Mode";  //0:MONO, 1:STEREO
  control[3].mode = CM_SELECTOR;
  control[3].levelCount = 2; 
  control[3].value = 0;

  control[4].name = "Dry-Wet Balance";
  control[4].mode = CM_POT;
  control[4].levelCount = 128;
  control[4].value = 64;

  control[5].name = "Output Mode";  //0:MONO, 1:STEREO
  control[5].mode = CM_SELECTOR;
  control[5].levelCount = 2; 
  control[5].value = 1;

  //setup the buttons
  encoderMode = EM_BUTTONS;
  
  //main button
  button[0].mode = BM_TOGGLE;
  
  //tap tempo button
  button[1].mode = BM_TAPTEMPO;
  button[1].min = 10;
  button[1].max = 2000;
  button[1].value = 500;

  //allocate the buffer at PSRAM
  //buffer length for 2 seconds delay at 44100 samples/second
#define BUFFER_LENGTH 88200
  delayBufferL = (float*)ps_malloc(BUFFER_LENGTH * sizeof(float));
  delayBufferR = (float*)ps_malloc(BUFFER_LENGTH * sizeof(float));

  //init the buffer with zero data
  for(int i=0;i<BUFFER_LENGTH;i++)
  {
    delayBufferL[i]=0;
    delayBufferR[i]=0;
  }

  //initialization
  writeIndex = 0;
  readIndex = 0;
  setTime(500);
  auxLed->blink(10,490,1,0,0);
  feedbackGain = 0.5;
  dryGain = 1;
  wetGain = 1;
} 

////////////////////////////////////////////////////////////////////////
void taptempoDelay::deInit()
{
  free(delayBufferL);
  free(delayBufferR);
}

////////////////////////////////////////////////////////////////////////
void taptempoDelay::onControlChange(int controlIndex)
{
  Serial.printf("%s : %d\r\n",control[controlIndex].name.c_str(),control[controlIndex].value);
  switch(controlIndex)
  {
    case 0: //time
    {
      int dt = (control[0].value +1)*10;
      setTime(dt); 
      auxLed->blinkUpdate(10,dt-10,1,0,0);
      break;
    }
    case 2: //repeat
    {
      feedbackGain = (float)control[2].value /127.0;
      break;
    }
    case 3: //input mode
    {
      if(button[0].value==0) //if bypassed
        if(control[3].value == 0) //if mono input
          setOutMix(false,true); //set the right output to LR mix
        else 
          setOutMix(false,false); //set the right output to LR mix
      break;
    }
    case 4: //dry-wet balance
    {
      dryGain =  2*(127.0-(float)control[4].value)/127;
      if(dryGain>1) dryGain=1;
      wetGain = 2*(float)control[4].value/127;
      if(wetGain>1) wetGain=1;
      break;
    }
  }
}

void taptempoDelay::onButtonChange(int buttonIndex)
{
   switch(buttonIndex)
   {
    case 0: //main button
    {
      if(button[0].value) //if effect is activated
      {
        analogSoftBypass(false);
        mainLed->turnOn();
        setOutMix(false,false); //normalize stereo separation in case of mixed at bypass
      }
      else //if effect is bypassed
      {
        analogSoftBypass(true); 
        mainLed->turnOff();
        if(control[3].value == 0) //if mono input
          setOutMix(false,true); //set the right output to LR mix
      }
      break;
    }
    case 1: //aux button (tap tempo)
    {
      setTime(button[1].value);
      auxLed->blink(10,button[1].value-10,1,0,0);
      break;
    }
   }
};
  
////////////////////////////////////////////////////////////////////////
void taptempoDelay::setTime(int ms)
{
  indexShift = (SAMPLE_RATE * ms)/1000;
  if(indexShift >= BUFFER_LENGTH)
    indexShift = BUFFER_LENGTH -1;
}
  
////////////////////////////////////////////////////////////////////////
void taptempoDelay::updateIndex()
{
  //compute the write index
  writeIndex++;
  if(writeIndex >= BUFFER_LENGTH)
    writeIndex = 0;
  
  //filter the indexShift into the filteredIndexShift to avoid index jump
  int delta = indexShift - filteredIndexShift;
  if(delta!=0)
  {
    int inc = delta/4410;
    if(inc == 0)
      inc = 1;
    filteredIndexShift += inc;
  }

  readIndex = writeIndex - filteredIndexShift;
  if(readIndex < 0)
    readIndex = BUFFER_LENGTH + readIndex;
  if(readIndex >= BUFFER_LENGTH)
    readIndex = readIndex-BUFFER_LENGTH;
}
  
////////////////////////////////////////////////////////////////////////
void taptempoDelay::process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount)
{   
  for(int i=0;i<sampleCount;i++)
  {
    updateIndex();

    float outL;
    float outR;

    if(control[3].value==0) //mono input
    {
      inRight[i] = inLeft[i];
    }

    if(control[5].value==1) //stereo output
    {
      outL = dryGain * inLeft[i] + wetGain * delayBufferL[readIndex];
      outR = dryGain * inRight[i] + wetGain * delayBufferR[readIndex];
    }
    else  //mono output
    {
      outL =  dryGain * (inLeft[i]+inRight[i]) + wetGain * (delayBufferL[readIndex] + delayBufferR[readIndex]);
      outL = outL/2;
      outR = outL;  //just copy the identical left output to the right output
    }

    delayBufferL[writeIndex] = feedbackGain * delayBufferR[readIndex] + inLeft[i];;
    if(control[3].value==0 && control[5].value==1)  //if mono input and stereo output
      delayBufferR[writeIndex] = feedbackGain * delayBufferL[readIndex];
    else 
      //stereo input or mono output
      delayBufferR[writeIndex] = feedbackGain * delayBufferL[readIndex] + inRight[i];
 
    outLeft[i]=outL;
    outRight[i]=outR;
  }
}

//declare an instance of your effect module
taptempoDelay  myPedal;

//setup the effect modules by calling blackstompSetup() inside arduino core's setup()
void setup() {
 //Serial communication
  Serial.begin(9600);

  //SETTING UP THE EFFECT MODULE
  blackstompSetup(&myPedal);
}

//do repetitive task here (for debugging info only)
void loop() {
  //Uncomment to show the system info (might introduce some audible noise on the output)
  /*
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
  */
  vTaskDelay(1000);
}
