#include <blackstomp.h>

//effect module class definition
class gainDoubler:public effectModule 
{
  private:
  float gain;
  float gainRange;
  
  public:
  void init();
  void deInit();
  void onButtonChange(int buttonIndex);
  void onControlChange(int controlIndex);
  void process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount);
};

//effect module class implementation
void gainDoubler::init()  
{
  //define your effect name
  name = "GAIN DOUBLER";

  //define the input mode (IM_LR or IM_LMIC) 
  inputMode = IM_LR;

  //setting up the buttons
  button[0].mode = BM_TOGGLE;
  encoderMode = EM_BUTTONS;
  button[1].mode = BM_TAPTEMPO;
  button[1].min = 50;
  button[1].max = 2000;
  button[2].mode = BM_MOMENTARY;

  control[0].mode = CM_POT;
  control[1].mode = CM_SELECTOR;
  control[1].levelCount = 3;

  gain = 1;
  gainRange = 1;
}

void gainDoubler::deInit()
{
  //do all the necessary deinitialization here
}

void gainDoubler::onButtonChange(int buttonIndex)
{
  switch(buttonIndex)
  {
    case 0: //main button state has changed
    {
      if(button[0].value) //if effect is activated
      {
        analogBypass(false);
        mainLed->turnOn();
      }
      else //if effect is bypassed
      {
        analogBypass(true); 
        mainLed->turnOff();
      }
      break;
    }
    case 1: //the button[1] state has changed
    {
      auxLed->blink(10,button[1].value-10,1,0,0);
      break;
    }
    case 2: //the button[0] state has changed
    {
      //do something here
      break;
    }
  }
}

void gainDoubler::onControlChange(int controlIndex)
{
  switch(controlIndex)
  {
    case 0:
    {
      gain = (float)control[0].value/127.0;
      break;
    }
    case 1:
    {
      if(control[1].value==0)
        gainRange = 0.5;
      else if(control[1].value==1)
        gainRange = 1;
      else gainRange = 2;
      break;
    }
  }
}

void gainDoubler::process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount)
{
  for(int i=0;i<sampleCount;i++)
  {
    outLeft[i] = gain * gainRange * inLeft[i];
    outRight[i] = gain * gainRange * inRight[i];
  }
}

//Arduino core setup
//declare an instance of your effect module
gainDoubler myPedal;
void setup()
{
  blackstompSetup(&myPedal);
}

//Arduino core loop
void loop()
{

}