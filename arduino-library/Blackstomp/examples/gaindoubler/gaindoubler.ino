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
  //select the appropriate device by uncommenting one of the following two lines:
  //setDeviceType(DT_ESP32_A1S_AC101);
  setDeviceType(DT_ESP32_A1S_ES8388);
  
  //default optimization for ES8388-version module  is 1/4 Vrms range
  //to optimize for the 1 Vrms range (more noisy), uncomment the following line:
  //optimizeConversion(0);
  
  //define your effect name
  name = "GAIN DOUBLER";
 
  //define the input mode (IM_LR or IM_LMIC) 
  inputMode = IM_LR;
 
  //setting up the buttons
  //setup the first button as toggle button
  button[0].mode = BM_TOGGLE;
  
  //enable encoder port for buttons
  encoderMode = EM_BUTTONS;
 
  //setup the second button as tap tempo button
  button[1].mode = BM_TAPTEMPO;
  button[1].min = 50;
  button[1].max = 2000;
 
  //setup the third button as temporary button
  button[2].mode = BM_MOMENTARY;

  //add gain control
  control[0].name = "Gain";
  control[0].mode = CM_POT;
  control[0].levelCount = 128;
  control[0].slowSpeed = true;

  //add range control
  control[1].name = "Range";
  control[1].mode = CM_SELECTOR;
  control[1].levelCount = 3;

  //add range conversion control
  control[2].name = "Conversion Optimizer";
  control[2].mode = CM_SELECTOR;
  control[2].levelCount = 5;

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
    case 2:
    {
      optimizeConversion(control[2].value);
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
  //setting up the effect module
  blackstompSetup(&myPedal);

  //run system monitor at 38400 baud rat, at 2000 ms update period
  //don't call this function when MIDI is implemented
  //try lowering the baudrate if audible noise is introduced on some boards
  runSystemMonitor(38400, 2000);
}

//Arduino core loop
//let the main loop empty to dedicate the core 1 for the main audio task
void loop() 
{

}
