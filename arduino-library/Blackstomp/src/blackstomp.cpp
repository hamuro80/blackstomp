/*!
 *  @file       blackstomp.cpp
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
#include "ac101.h"
#include "driver/i2s.h"
#include "esp_task_wdt.h"
#include "math.h"
#include "EEPROM.h"

//CONTROL INPUT
#define P1_PIN  39
#define P2_PIN  36
#define P3_PIN  34
#define P4_PIN  14
#define P5_PIN  13
#define P6_PIN  4

//ROTARY ENCODER
#define RE_BUTTON_PIN 19
#define RE_PHASE0_PIN 23
#define RE_PHASE1_PIN 18

//FOOT SW PIN SETUP
#define FS_PIN 5

//LED INDICATOR PIN SETUP
#define MAINLED_PIN 2
#define AUXLED_PIN 15

//OLED Display PIN SETUP
#define SCK_PIN 22
#define SDA_PIN 21

//AC101 AUDIO CODEC PIN SETUP
#define I2S_NUM         (0)
#define I2S_MCLK        (GPI_NUM_0)
#define I2S_BCK_IO      (GPIO_NUM_27)
#define I2S_WS_IO       (GPIO_NUM_26)
#define I2S_DO_IO       (GPIO_NUM_25)
#define I2S_DI_IO       (GPIO_NUM_35)
#define CODEC_SCK       (GPIO_NUM_32)
#define CODEC_SDA       (GPIO_NUM_33)

//audio processing frame length in samples (L+R) 64 samples (32R+32L) 256 Bytes
#define FRAMELENGTH    64
//sample count per channel for each frame (32)re
#define SAMPLECOUNT   FRAMELENGTH/2
//channel count inside a frame (always stereo = 2)
#define CHANNELCOUNT  2
//frame size in bytes
#define FRAMESIZE   FRAMELENGTH*4
//audio processing priority
#define AUDIO_PROCESS_PRIORITY  10

//dma buffer length 32 bytes (8 samples: 4L+4R)
#define DMABUFFERLENGTH 32
//dma buffer count 20 (640 Bytes: 160 samples: 80L+80R) 
#define DMABUFFERCOUNT  20

//codec instance
static AC101 _codec;

//effect module pointer
static effectModule* _module = NULL;
bool _codecIsReady = false;

//controlInterface pointer
static controlInterface _control;

//led indicator
static ledIndicator _mainLed;
static ledIndicator _auxLed;

//BLE terminal
static bt_terminal* btt;

//buffers
static float wleft[] = {0,0};
static float wright[] = {0,0};
static int32_t inbuffer[FRAMELENGTH];
static int32_t outbuffer[FRAMELENGTH];
static float inleft[FRAMESIZE];
static float inright[FRAMESIZE];
static float outleft[FRAMESIZE];
static float outright[FRAMESIZE];

static unsigned int usedticks;
static unsigned int availableticks;
static unsigned int availableticks_start;
static unsigned int availableticks_end;
static unsigned int usedticks_start;
static unsigned int usedticks_end;
static volatile unsigned int processedframe;
static unsigned int audiofps;

static unsigned long eepromupdatetime = 0;
static bool eepromrequestupdate = false;
struct EEPROMBUFFER
{
  int controlvalue[6];
  int buttonvalue[4];
};
static EEPROMBUFFER eeprombuffer;

void framecounter_task(void* arg)
{
  processedframe = 0;
  while(true)
  {
    audiofps = processedframe;
    processedframe = 0;
    vTaskDelay(1000);
  }
  vTaskDelete(NULL);
}

void i2s_task(void* arg)
{
  size_t bytesread, byteswritten;

  //initialize all output buffer to zero
  for(int i= 0; i< FRAMELENGTH; i++)
    outbuffer[i] = 0;
    
  for(int i=0; i< SAMPLECOUNT; i++)
  {
    outleft[i]=0;
    outright[i]=0;
  }

  usedticks_start = xthal_get_ccount();
  availableticks_start = xthal_get_ccount();
  
  while(true)
  {
    availableticks_end = xthal_get_ccount();
    availableticks = availableticks_end - availableticks_start;
    availableticks_start = availableticks_end;
    
    i2s_read((i2s_port_t)I2S_NUM,(void*) inbuffer, FRAMESIZE, &bytesread, 20);

    //used-tick counter starting point
    usedticks_start = xthal_get_ccount();

    if(_control.runningTicks < 1000) //silence the signal during the first 1000 ms startup
    {
      for(int i=0,k=0;k<SAMPLECOUNT;k++,i+=2)
      {
        inleft[k] = 0;  
        inright[k] = 0;
      }
    }
    else 
    for(int i=0,k=0;k<SAMPLECOUNT;k++,i+=2)
    {
      //convert to 24 bit int then to float
      inleft[k] = (float) (inbuffer[i]>>8);
      inright[k] = (float) (inbuffer[i+1]>>8);

      //scale to 1.0
      inleft[k] = inleft[k]/8388608;
      inright[k]=inright[k]/8388608;
    }
  
    //process the signal by the effect module
    _module->process(inleft, inright, outleft, outright, SAMPLECOUNT);
    processedframe++;
    
    //convert back float to int
    for(int i=0,k=0;k<SAMPLECOUNT;k++,i+=2)
    {
      //scale the left output to 24 bit range
      outleft[k] = outleft[k]*8388607;
      //saturate to signed 24bit range
      if(outleft[k]>8388607) outleft[k]=8388607;
      if(outleft[k]<-8388607) outleft[k]= -8388607;

      //scale the right output to 24 bit range
      outright[k]=outright[k]*8388607;
      //saturate to signed 24bit range
      if(outright[k]>8388607) outright[k]=8388607;
      if(outright[k]<-8388607) outright[k]= -8388607;
      
      //convert to 32 bit int
      outbuffer[i] = ((int32_t) outleft[k])<<8;
      outbuffer[i+1] = ((int32_t) outright[k])<<8;
    }

    //used-tick counter end point
    usedticks_end = xthal_get_ccount();
    usedticks = usedticks_end - usedticks_start;
    
    i2s_write((i2s_port_t)I2S_NUM,(void*) outbuffer, FRAMESIZE, &byteswritten, 20);
    esp_task_wdt_reset();
  }
  vTaskDelete(NULL);
}

void i2s_setup()
{
  i2s_config_t i2s_config;
  i2s_config.mode =(i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX);
  i2s_config.sample_rate = SAMPLE_RATE;
  i2s_config.bits_per_sample = (i2s_bits_per_sample_t) 32;
  i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT; //both channel
  i2s_config.communication_format = (i2s_comm_format_t) I2S_COMM_FORMAT_I2S;
  
  i2s_config.dma_buf_count = DMABUFFERCOUNT;
  i2s_config.dma_buf_len = DMABUFFERLENGTH;

#ifdef USE_APLL_MCLK_6M
  i2s_config.use_apll = true;
  i2s_config.fixed_mclk = 6000000; 
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
  WRITE_PERI_REG(PIN_CTRL, READ_PERI_REG(PIN_CTRL) & 0xFFFFFFF0);
#else
  i2s_config.use_apll = false;
  i2s_config.fixed_mclk = 0; 
#endif
  
  i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1 ;
  i2s_driver_install((i2s_port_t)I2S_NUM, &i2s_config, 0, NULL);
  
  i2s_pin_config_t pin_config;
  pin_config.bck_io_num = I2S_BCK_IO;
  pin_config.ws_io_num = I2S_WS_IO;
  pin_config.data_out_num = I2S_DO_IO;
  pin_config.data_in_num = I2S_DI_IO;
  
  i2s_set_pin((i2s_port_t)I2S_NUM, &pin_config);
  i2s_set_clk((i2s_port_t)I2S_NUM, SAMPLE_RATE, (i2s_bits_per_sample_t) 32, I2S_CHANNEL_STEREO);
}

void button_task(void* arg)
{
  //state variables
  int bpin[] = {FS_PIN, RE_BUTTON_PIN, RE_PHASE0_PIN, RE_PHASE1_PIN};
  int bstate[4];
  int bstatecounter[4];
  
  //initialize pin mode
  pinMode(FS_PIN,INPUT_PULLUP);
  
  if(_module->encoderMode != EM_DISABLED)
  {
    pinMode(RE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(RE_PHASE0_PIN, INPUT_PULLUP);
    pinMode(RE_PHASE1_PIN, INPUT_PULLUP);
  }

  for(int i=0;i<4;i++)
  {
    bstate[i]=0;
    bstatecounter[i] = 0;
  }
  static int bcount = 1;
  if(_module->encoderMode == EM_BUTTONS)
    bcount = 4;
  static int tempocounter = 0;
  while(true)
  {
    vTaskDelay(1);
    for(int i=0;i<bcount;i++)
    {
      int temp = !digitalRead(bpin[i]);
      if(_module->button[i].inverted)
        temp = !temp;
        
      if(_module->button[i].mode == BM_TOGGLE)
      {
        if(temp!= bstate[i])
        {
          bstatecounter[i]++;
          if(bstatecounter[i]>9) //debouncing
          {
            bstate[i] = temp;
            bstatecounter[i]=0;
            if(temp)
            {
              _module->onButtonPress(i);
              if(_module->button[i].value == 1)
                _module->button[i].value = 0;
              else
                _module->button[i].value = 1;
                
              _module->onButtonChange(i);
              eepromrequestupdate = true;
            }
            else
            {
              _module->onButtonRelease(i);
            }
          }
        }
        else bstatecounter[i]=0;
      }
      else if(_module->button[i].mode == BM_MOMENTARY)
      {
        if(temp != _module->button[i].value)
        {
          bstatecounter[i]++;
          if(bstatecounter[i]>9) //debouncing
          {
            bstatecounter[i]=0;
            if(temp==0)
            {
              _module->onButtonRelease(i);
              _module->button[i].value = 0;
              _module->onButtonChange(i);
            }
            else //temp=1
            {
              _module->onButtonPress(i);
              _module->button[i].value = 1;
              _module->onButtonChange(i);
            }
          }
        }
        else //temp = _module->button[i].value
        {
          bstatecounter[i]=0;
        }
      }
      else if(_module->button[i].mode == BM_TAPTEMPO)
      {
        switch(bstate[i])
        {
          case 0: //wait first tap
          {
            if(temp) //tapped
            {
              bstate[i] = 1; //wait release, no need to wait stable press
              tempocounter = 0; //start tempo couter immediately
              _auxLed.turnOn();
            }
            break;
          }
          case 1: //wait release
          {
            tempocounter++;
            if(tempocounter > _module->button[i].max) //if expired
            {
              _auxLed.blink(10,_module->button[i].value-10,1,0,0);
              bstate[i]=0; //back to start
            }
            else if(!temp) //released
            {
              bstate[i] = 2; //wait stable release
              bstatecounter[i]=0;
            }
            break;
          }
          case 2: //wait stable release
          {
            tempocounter++;
            if(tempocounter > _module->button[i].max) //if expired
            {
              _auxLed.blink(10,_module->button[i].value-10,1,0,0);
              bstate[i]=0; //back to start
            }
            else if(!temp) //if released
            {
              bstatecounter[i]++;
              if(bstatecounter[i] > 10)
              {
                bstate[i]=3; //wait second tap
              }
            }
            else //not release
              bstatecounter[i] = 0; //reset the stable count
            break;
          }
          case 3: //wait second tap
          {
            tempocounter++;
            if(tempocounter > _module->button[i].max) //if expired
            {
              _auxLed.blink(10,_module->button[i].value-10,1,0,0);
              bstate[i]=0; //back to start
            }
            else if(temp) //tapped
            {
                if(tempocounter < _module->button[i].min)
                  tempocounter = _module->button[i].min;
                _auxLed.blink(10,tempocounter-10,1,0,0);
                _module->button[i].value = tempocounter;
                _module->onButtonChange(i);
                
              bstatecounter[i] = 0;
              bstate[i] = 4; //wait the second stable release
            }
            break;
          }
          case 4: //wait the second  stable release
          {
            if(!temp) //released
            {
              bstatecounter[i]++;
              if(bstatecounter[i] > 10)
              {
                bstate[i] = 0; //back to wait the first tap
              }
            }
            else bstatecounter[i]=0;
            
            break;
          }
        }//end switch
      }
    }
  }
}

void codecsetup_task(void* arg)
{
  _codec.setup(CODEC_SDA, CODEC_SCK);
  _codecIsReady = true;

  //setting up the input mode
  if(_module->inputMode == IM_LR)
  {
    _codec.LeftLineLeft(true);
    _codec.RightLineRight(true);
  }
  else
  {
    _codec.LeftLineLeft(true);
    _codec.RightMic1(true);
  }
  
  vTaskDelete(NULL);
}

void eepromupdate_task(void* arg)
{
  while(true)
  {
    uint8_t* pByte = (uint8_t*) &eeprombuffer;
    if(eepromrequestupdate || _control.unsavedchanges)
    {
      //copy the control value to eeprombuffer
      for(int i=0;i<6;i++)
          eeprombuffer.controlvalue[i]=_module->control[i].value;

      //copy the button value to eeprombuffer
      for(int i=0;i<4;i++)
          eeprombuffer.buttonvalue[i]=_module->button[i].value;

      //write the eeprombuffer to eeprom
      for(int i=0;i<sizeof(eeprombuffer);i++)
      {
        EEPROM.write(i,pByte[i]);
      }
      EEPROM.commit();
      eepromrequestupdate = false;
      _control.unsavedchanges = false;
      Serial.printf("EEPROM update!\n");
    }
    vTaskDelay(2000);
  }
}

void eepromsetup_task(void* arg)
{
  if (!EEPROM.begin(sizeof(eeprombuffer)))
  {
    Serial.printf("Failed to initialise EEPROM\n");
  }
  else
  {
    //load the eeprombuffer from EEPROM
    uint8_t* pByte = (uint8_t*) &eeprombuffer;
    for(int i=0;i<sizeof(eeprombuffer);i++)
    {
      pByte[i]=EEPROM.read(i);
    }

    //load control values from buffer
    vTaskDelay(1000);
    for(int i=0;i<6;i++)
    {
      if(_module->control[i].mode == CM_TOGGLE)
        _module->control[i].value = eeprombuffer.controlvalue[i];
        
      //call all enabled control callbacks at first run
      if(_module->control[i].mode != CM_DISABLED)
        _module->onControlChange(i);
    }
    
    //load buton values from buffer
    for(int i=0;i<4;i++)
    {
      if(_module->button[i].mode == BM_TOGGLE)
      {
        _module->button[i].value = eeprombuffer.buttonvalue[i];
        _module->onButtonChange(i);
      }
    }

  }
  
  xTaskCreatePinnedToCore(eepromupdate_task, "eepromupdate_task", 4096, NULL, AUDIO_PROCESS_PRIORITY, NULL,0);
  vTaskDelete(NULL);
}

void blackstompSetup(effectModule* module) 
{
  //init the LED indicator
  _mainLed.init(MAINLED_PIN, AUDIO_PROCESS_PRIORITY-1);
  _auxLed.init(AUXLED_PIN, AUDIO_PROCESS_PRIORITY-1);
  
  //assign the module pointer "_module" and init the module
  _module = module;
  _module->auxLed = &_auxLed;
  _module->mainLed = &_mainLed;
  _module->init();
  
  //validate the port setting
  for(int i=0;i<6;i++)
  {
	  switch(_module->control[i].mode)
	  {
		  case CM_POT:
		  {
			  if(_module->control[i].levelCount > 256)
				_module->control[i].levelCount = 256;
			  if(_module->control[i].levelCount < 2)
				_module->control[i].levelCount = 2;
			  _module->control[i].min = 0;
			  _module->control[i].max = _module->control[i].levelCount -1;
			  break;
		  }
		  case CM_SELECTOR:
		  {
			  if(_module->control[i].levelCount > 12)
				_module->control[i].levelCount = 12;
			  if(_module->control[i].levelCount < 2)
				_module->control[i].levelCount = 2;
			  _module->control[i].min = 0;
			  _module->control[i].max = _module->control[i].levelCount -1;
			  break;
		  }
		  case CM_TAPTEMPO:
		  {
			  if(_module->control[i].min < 50)
				_module->control[i].min = 50;
			  break;
		  }
	  }
  }

  //codec setup
  xTaskCreatePinnedToCore(codecsetup_task, "codecsetup_task", 4096, NULL, AUDIO_PROCESS_PRIORITY-1, NULL,0);

  //assign the module to control and start it
  _control.module = _module;
  _control.init(P1_PIN,P2_PIN,P3_PIN,P4_PIN,P5_PIN,P6_PIN,AUDIO_PROCESS_PRIORITY-1);
  
  //decoding button press on main button port and encoder port
  xTaskCreatePinnedToCore(button_task, "button_task", 4096, NULL, AUDIO_PROCESS_PRIORITY-1, NULL,0);

  i2s_setup();
  //the main audio task, dedicated on core 1
  xTaskCreatePinnedToCore(i2s_task, "i2s_task", 4096, NULL, AUDIO_PROCESS_PRIORITY, NULL,1);
  
  //audio frame moitoring task
  xTaskCreatePinnedToCore(framecounter_task, "i2s_task", 4096, NULL, AUDIO_PROCESS_PRIORITY, NULL,0);
  
  //run eeprom service to manage saving some parameter control change in limited update frequency to save the flash from aging
  xTaskCreatePinnedToCore(eepromsetup_task, "eepromsetup_task", 4096, NULL, AUDIO_PROCESS_PRIORITY-1, NULL,0);
}

void enableBleTerminal(void)
{
	btt = new bt_terminal();
	btt->module = _module;
	
	uint64_t mac; //8 bytes wide
	esp_efuse_mac_get_default((uint8_t*)&mac); //getting 6 bytes mac
	mac = mac >> 16;  //shift 2 bytes
	
	char macstr[15];
	sprintf(macstr,"%lX",mac);
	String dname = _module->name;
	dname = dname.substring(0,14) + (String) " - " + (String) macstr;

	const char* su;
	const char* cu;
	su = _module->bleTerminal.servUuid.c_str();
	cu = _module->bleTerminal.charUuid.c_str();
	btt->begin(dname.c_str(),su,cu,_module->bleTerminal.passKey,10);
}

bool analogBypass(bool bypass)
{
    if(!_codecIsReady)
      return false;

    //connect or disconnect the DAC output to output mixer
    _codec.OmixerRightDacRight(!bypass);
    _codec.OmixerLeftDacLeft(!bypass);

    //connect or disconnect the ADC input
     if(_module->inputMode == IM_LR)
    {
      _codec.LeftLineLeft(!bypass);
      _codec.RightLineRight(!bypass);
    }
    else
    {
      _codec.LeftLineLeft(!bypass);
      _codec.RightMic1(!bypass);
    }

    //connect/disconnect the line/mic input to output mixer
    if(_module->inputMode == IM_LR)
    {      
      _codec.OmixerLeftLineLeft(bypass);
      _codec.OmixerRightLineRight(bypass);

    }
    else //inputMode = IM_LMIC
    {
       _codec.OmixerRightMic1(bypass);
       _codec.OmixerLeftLineLeft(bypass);
    }
    return true;
}

bool analogSoftBypass(bool bypass)
{
    if(!_codecIsReady)
      return false;
      
    //connect or disconnect the ADC input
     if(_module->inputMode == IM_LR)
    {
      _codec.LeftLineLeft(!bypass);
      _codec.RightLineRight(!bypass);
    }
    else
    {
      _codec.LeftLineLeft(!bypass);
      _codec.RightMic1(!bypass);
    }

    //connect/disconnect the line/mic input to output mixer
    if(_module->inputMode == IM_LR)
    {      
      _codec.OmixerLeftLineLeft(bypass);
      _codec.OmixerRightLineRight(bypass);

    }
    else //inputMode = IM_LMIC
    {
       _codec.OmixerRightMic1(bypass);
       _codec.OmixerLeftLineLeft(bypass);
    }
    return true;
}

int getTotalCpuTicks()
{
  return availableticks;
}

int getUsedCpuTicks()
{
  return usedticks;
}

float getCpuUsage()
{
  return (float)usedticks/(float)availableticks;
}

int getAudioFps()
{
  return audiofps;
}

void setMicGain(int gain)
{
  _codec.SetMicGain(gain);
}

int getMicGain()
{
  return _codec.GetMicGain();
}

void setOutVol(int vol)
{
  _codec.SetVolSpeaker(vol);
}

bool setOutMix(bool mixedLeft, bool mixedRight)
{
	return _codec.SetOutputMode(mixedLeft,mixedRight);
}

int getOutVol()
{
  return _codec.GetVolSpeaker();
}
