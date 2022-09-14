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
//#include "ac101.h"
#include "driver/i2s.h"
#include "esp_task_wdt.h"
#include "math.h"
#include "EEPROM.h"
#include "codec.h"

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

//Digital input Generic Assignment
#define D1_PIN_AC101	FS_PIN
#define D2_PIN_AC101	RE_BUTTON_PIN
#define D3_PIN_AC101	RE_PHASE0_PIN
#define D4_PIN_AC101	RE_PHASE1_PIN

#define D1_PIN_ES8388	RE_BUTTON_PIN
#define D2_PIN_ES8388	SCK_PIN
#define D3_PIN_ES8388	SDA_PIN


//ESP32-A1S-AC101 PIN SETUP
#define I2S_NUM         (0)
#define I2S_MCLK        (GPI_NUM_0)
#define I2S_BCK_IO      (GPIO_NUM_27)
#define I2S_WS_IO       (GPIO_NUM_26)
#define I2S_DO_IO       (GPIO_NUM_25)
#define I2S_DI_IO       (GPIO_NUM_35)

#define AC101_SDA		(GPIO_NUM_33)
#define AC101_SCK		(GPIO_NUM_32)
#define AC101_ADDR 		0x1A

//ESP32-A1S-ES8388 PIN SETUP
#define I2S_BCK_IO_ES	(GPIO_NUM_5)
#define I2S_WS_IO_ES    (GPIO_NUM_25)
#define I2S_DI_IO_ES    (GPIO_NUM_35)
#define I2S_DO_IO_ES     (GPIO_NUM_26)

#define ES8388_SDA		(GPIO_NUM_18)
#define ES8388_SCK		(GPIO_NUM_23)
#define ES8388_ADDR		0x10

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
//#define DMABUFFERCOUNT  20
//dma buffer count 2 (for 2 ms measured latency)
#define DMABUFFERCOUNT  2

//codec instance
//static AC101 _codec;
static codec* _acodec;
//static bool _es8388Mode = false;
DEVICE_TYPE _deviceType = DT_ESP32_A1S_AC101;
static uint8_t _codecAddress = 0;
static bool _muteLeftAdcIn = false;
static bool _muteRightAdcIn = false;

//effect module pointer
static effectModule* _module = NULL;
bool _codecIsReady = false;
static float _outCorrectionGain = 1;
static int _optimizedRange = 2;


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
static char* debugStringPtr = "None";
static float debugVars[]={0,0,0,0};

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

void setDebugStr(const char* str)
{
	debugStringPtr = (char*) str;
}

void setDeviceType(DEVICE_TYPE dt)
{
	_deviceType = dt;
}

void setDebugVars(float val1, float val2, float val3, float val4)
{
	debugVars[0]=val1;
	debugVars[1]=val2;
	debugVars[2]=val3;
	debugVars[3]=val4;
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
      for(int k=0;k<SAMPLECOUNT;k++)
      {
        inleft[k] = 0;  
        inright[k] = 0;
      }
    }
    else 
    for(int i=0,k=0;k<SAMPLECOUNT;k++,i+=2)
    {
		if(!_muteLeftAdcIn)
		{
		  //convert to 24 bit int then to float
		  inleft[k] = (float) (inbuffer[i]>>8);
		  //scale to 1.0
		  inleft[k] = inleft[k]/8388608;
		}
		else inleft[k]=0;
		
		if(!_muteRightAdcIn)
		{
		  //convert to 24 bit int then to float
		  inright[k] = (float) (inbuffer[i+1]>>8);
		  //scale to 1.0
		  inright[k]=inright[k]/8388608;
		}
		else inright[k] = 0;
    }
  
    //process the signal by the effect module
    _module->process(inleft, inright, outleft, outright, SAMPLECOUNT);
    processedframe++;
    
    //convert back float to int
    for(int i=0,k=0;k<SAMPLECOUNT;k++,i+=2)
    {
      //scale the left output to 24 bit range
      outleft[k] = _outCorrectionGain * outleft[k] * 8388607;
      //saturate to signed 24bit range
      if(outleft[k]>8388607) outleft[k]=8388607;
      if(outleft[k]<-8388607) outleft[k]= -8388607;

      //scale the right output to 24 bit range
      outright[k]=_outCorrectionGain * outright[k] * 8388607;
      //saturate to signed 24bit range
      if(outright[k]>8388607) outright[k]=8388607;
      if(outright[k]<-8388607) outright[k]= -8388607;
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

	if(_deviceType==DT_ESP32_A1S_ES8388)
	{
		i2s_config.use_apll = true;
		i2s_config.fixed_mclk = 33868800;	//double speed mclk/lrck ratio = 384
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
		WRITE_PERI_REG(PIN_CTRL, READ_PERI_REG(PIN_CTRL) & 0xFFFFFFF0);
	}
	else if(_deviceType == DT_ESP32_A1S_AC101)
	{
		i2s_config.use_apll = false;
		i2s_config.fixed_mclk = 0; 
	}

	i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1 ;
	i2s_driver_install((i2s_port_t)I2S_NUM, &i2s_config, 0, NULL);

	i2s_pin_config_t pin_config;
	if(_deviceType == DT_ESP32_A1S_ES8388)
	{
	  pin_config.bck_io_num = I2S_BCK_IO_ES;
	  pin_config.ws_io_num = I2S_WS_IO_ES;
	  pin_config.data_out_num = I2S_DO_IO_ES;
	  pin_config.data_in_num = I2S_DI_IO_ES;
	}
	else if(_deviceType==DT_ESP32_A1S_AC101)
	{
	  pin_config.bck_io_num = I2S_BCK_IO;
	  pin_config.ws_io_num = I2S_WS_IO;
	  pin_config.data_out_num = I2S_DO_IO;
	  pin_config.data_in_num = I2S_DI_IO;
	}

	i2s_set_pin((i2s_port_t)I2S_NUM, &pin_config);
	i2s_set_clk((i2s_port_t)I2S_NUM, SAMPLE_RATE, (i2s_bits_per_sample_t) 32, I2S_CHANNEL_STEREO);
}

void button_task(void* arg)
{
  //state variables
  int bpin[4];
  int bstate[4];
  int bstatecounter[4];
  
	if(_deviceType == DT_ESP32_A1S_ES8388)
	{
		bpin[0]= D1_PIN_ES8388;
		bpin[1]= D2_PIN_ES8388;
		bpin[2]= D3_PIN_ES8388;
		
		//disable the 4th button function
		_module->button[3].mode = BM_DISABLED;
		
		//initialize pin mode
		pinMode(bpin[0],INPUT_PULLUP); //main foot switch button
		if(_module->encoderMode == EM_BUTTONS)
		{
			pinMode(bpin[1], INPUT_PULLUP);
			pinMode(bpin[2], INPUT_PULLUP);
		}
	}
	else if(_deviceType == DT_ESP32_A1S_AC101)
	{
		bpin[0]=D1_PIN_AC101;
		bpin[1]=D2_PIN_AC101;
		bpin[2]=D3_PIN_AC101;
		bpin[3]=D4_PIN_AC101;
		
		//initialize pin mode
		pinMode(bpin[0],INPUT_PULLUP);
		if(_module->encoderMode == EM_BUTTONS)
		{
			pinMode(bpin[1], INPUT_PULLUP);
			pinMode(bpin[2], INPUT_PULLUP);
			pinMode(bpin[3], INPUT_PULLUP);
		}
	}

  for(int i=0;i<4;i++)
  {
    bstate[i]=0;
    bstatecounter[i] = 0;
  }
  
  static int bcount = 1;
	if(_module->encoderMode == EM_BUTTONS)
	{
		if(_deviceType==DT_ESP32_A1S_ES8388)
		{
			bcount = 3;
		}
		else if(_deviceType==DT_ESP32_A1S_AC101)
		{
			bcount = 4;
		}
	}
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
	if(_deviceType==DT_ESP32_A1S_AC101)
	{
		codecBusInit(AC101_SDA, AC101_SCK, 400000);
		_codecAddress = AC101_ADDR;
		
		_acodec = new AC101Codec();
		_acodec->setInputMode(_module->inputMode);
		bool res = _acodec->init(_codecAddress);
		_acodec->muteLeftAdcIn = &_muteLeftAdcIn;
		_acodec->muteRightAdcIn = &_muteRightAdcIn;
		_outCorrectionGain = _acodec->outCorrectionGain;
	}
	else if(_deviceType==DT_ESP32_A1S_ES8388)
	{
		codecBusInit(ES8388_SDA,ES8388_SCK, 32000);
		_codecAddress = ES8388_ADDR;
		
		_acodec = new ES8388Codec();
		_acodec->setInputMode(_module->inputMode);
		bool res = _acodec->init(_codecAddress);
		_acodec->muteLeftAdcIn = &_muteLeftAdcIn;
		_acodec->muteRightAdcIn = &_muteRightAdcIn;
		_outCorrectionGain = _acodec->outCorrectionGain;
	}
	_codecIsReady = true;
	optimizeConversion(_optimizedRange);

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
	
	//setup the i2S 
	i2s_setup();
	//the main audio task, dedicated on core 1
	xTaskCreatePinnedToCore(i2s_task, "i2s_task", 4096, NULL, AUDIO_PROCESS_PRIORITY, NULL,1);
	delay(100);

	//codec setup
	xTaskCreatePinnedToCore(codecsetup_task, "codecsetup_task", 4096, NULL, AUDIO_PROCESS_PRIORITY, NULL,0);

	//assign the module to control and start it
	_control.module = _module;
	_control.init(P1_PIN,P2_PIN,P3_PIN,P4_PIN,P5_PIN,P6_PIN,AUDIO_PROCESS_PRIORITY);

	//decoding button press on main button port and encoder port
	xTaskCreatePinnedToCore(button_task, "button_task", 4096, NULL, AUDIO_PROCESS_PRIORITY, NULL,0);

	//audio frame moitoring task
	xTaskCreatePinnedToCore(framecounter_task, "framecounter_task", 4096, NULL, AUDIO_PROCESS_PRIORITY, NULL,0);

	//run eeprom service to manage saving some parameter control change in limited update frequency to save the flash from aging
	xTaskCreatePinnedToCore(eepromsetup_task, "eepromsetup_task", 4096, NULL, AUDIO_PROCESS_PRIORITY, NULL,0);
}

void sysmon_task(void *arg)
{
	int* period = (int*)(arg);
	while(true)
	{
	  //System info
	  Serial.printf("\nSYSTEM INFO:\n");
	  Serial.printf("Internal Total heap %d, internal Free Heap %d\n",ESP.getHeapSize(),ESP.getFreeHeap());
	  Serial.printf("SPIRam Total heap %d, SPIRam Free Heap %d\n",ESP.getPsramSize(),ESP.getFreePsram());
	  Serial.printf("ChipRevision %d, Cpu Freq %d, SDK Version %s\n",ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
	  Serial.printf("Flash Size %d, Flash Speed %d\n",ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
	  
	  //Blackstomp application info
	  Serial.printf("\nAPPLICATION INFO:\n");
	  Serial.printf("Pedal Name: %s\n",_module->name.c_str());
	  Serial.printf("Audio frame per second: %d fps\n",getAudioFps());
	  Serial.printf("CPU ticks per frame period: %d\n",getTotalCpuTicks());
	  Serial.printf("Used CPU ticks: %d\n",getUsedCpuTicks());
	  Serial.printf("CPU Usage: %.2f %%\n", 100.0*getCpuUsage());
	  for(int i=0;i<6;i++)
	  {
		  if(_module->control[i].mode != CM_DISABLED)
			Serial.printf("CTRL-%d %s: %d\n",i,_module->control[i].name.c_str(),_module->control[i].value);
	  }
	  for(int i=0;i<4;i++)
	  {
		  if(_module->button[i].mode != CM_DISABLED)
			Serial.printf("BUTTON-%d: %d\n",i,_module->button[i].value);
	  }
	  char debugstring[51];
	  strncpy(debugstring,debugStringPtr,50);
	  Serial.printf("Debug String: %s\n", debugstring);
	  Serial.printf("Debug Variables: %g, %g, %g, %g\n", debugVars[0], debugVars[1], debugVars[2], debugVars[3]);
	  vTaskDelay(period[0]);
	}
	vTaskDelete(NULL);
}

int _updatePeriod;
void runSystemMonitor(int baudRate, int updatePeriod)
{
	Serial.begin(baudRate);
	_updatePeriod = updatePeriod;
	//run the performance monitoring task at 0 (idle) priority
	xTaskCreatePinnedToCore(sysmon_task, "sysmon_task", 4096, &_updatePeriod, 0, NULL,0);
}

enum {scp_disabled, scp_startWaitTrigger, scp_waitTrigger, scp_probing, scp_waitdisplay};
static int scpState = scp_disabled;

static int scSampleCount;
static int scSampleLength;
static int scTriggerChannel;
static float scPrevSample;
static float scTriggerLevel;
static bool scRisingTrigger;
static float** scData = NULL;

void scope_task(void *arg)
{
	scSampleCount = 0;
	scpState = scp_startWaitTrigger;
	while(true)
	{
		if(scpState==scp_waitdisplay)
		{
			//send the data to serial display
			for(int j=0;j<scSampleLength;j++)
			{
					Serial.printf("ch-%d:%f, ch-%d:%f\n", 0,scData[0][j],1,scData[1][j]);
			}
			Serial.flush();
			vTaskDelay(1);
			scpState = scp_startWaitTrigger;
		}	
		vTaskDelay(100);
	}
	vTaskDelete(NULL);
}

void runScope(int baudRate, int sampleLength, int triggerChannel, float triggerLevel, bool risingTrigger)
{
	Serial.begin(baudRate);
	scSampleLength = sampleLength;
	scTriggerLevel = triggerLevel;
	scRisingTrigger = risingTrigger;
	scTriggerChannel = triggerChannel;
	
	scData = new float*[2];
	for(int i=0;i<2;i++)
		scData[i]=(float*)ps_malloc((scSampleLength +1) * sizeof(float));
	for(int i=0;i<scSampleLength;i++)
	{
		scData[0][i] = 0;
		scData[1][i] = 0;
	}
	xTaskCreatePinnedToCore(scope_task, "scope_task", 4096, NULL, AUDIO_PROCESS_PRIORITY-1, NULL,0);
}

void scopeProbe(float sample, int channel)
{
	switch(scpState)
	{
		case scp_probing:
		{
			if(scSampleCount < scSampleLength)
			scData[channel][scSampleCount] = sample;
			if(channel == scTriggerChannel)
			{
				scSampleCount++;
				if(scSampleCount>= scSampleLength)
				{
					scpState = scp_waitdisplay;
				}
			}
			break;
		}
		case scp_startWaitTrigger:
		{
			if(channel == scTriggerChannel)
			{
				scPrevSample = sample;
				scpState = scp_waitTrigger;
			}
			break;
		}
		case scp_waitTrigger:
		{
			if(channel == scTriggerChannel)
			{
				if(scRisingTrigger)
				{
					if((sample > scTriggerLevel)&&(scPrevSample <= scTriggerLevel))
					{
						scSampleCount = 0;
						scpState = scp_probing;
					}
				}
				else 
				{
					if((sample < scTriggerLevel)&&(scPrevSample >= scTriggerLevel))
					{
						scSampleCount = 0;
						scpState = scp_probing;
					}
				}
				scPrevSample = sample;

			}
			break;
		}
	}
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

bool analogBypass(bool bypass, BYPASS_MODE bm)
{
    if(!_codecIsReady)
      return false;
    return _acodec->analogBypass(bypass, bm);
}

bool analogSoftBypass(bool bypass, BYPASS_MODE bm)
{
    if(!_codecIsReady)
      return false;
    return _acodec->analogSoftBypass(bypass, bm);
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
  //_codec.SetMicGain(gain);
  _acodec->setMicGain(gain);
}

int getMicGain()
{
  //return _codec.GetMicGain();
  return _acodec->getMicGain();
}

//set the input gain (analog gain) (0..8 for ES8388)
void setInGain(int gain)
{
	_acodec->setInGain(gain);
}

//optimize the analog to digital conversion range (0..4 for ES8388-version module)
void optimizeConversion(int range)
{
	if(_codecIsReady)
		_acodec->optimizeConversion(range);
	else _optimizedRange = range;
}

//set microphone noise gate (0-31: -76.5dB, -75.0dB,...., -30.0dB)
void setMicNoiseGate(int gate)
{ 
	_acodec->setMicNoiseGate(gate);
}

//get microphone noise gate (0-31: -76.5dB, -75.0dB,...., -30.0dB)
int getMicNoiseGate()
{
	_acodec->getMicNoiseGate();
}

//get the input gain (analog gain) (0..8 for ES8388)
int getInGain()
{
	return _acodec->getInGain();
}

void setOutVol(int vol)
{
  //_codec.SetVolSpeaker(vol);
  _acodec->setOutVol(vol);
}

int getOutVol()
{
  //return _codec.GetVolSpeaker();
  return _acodec->getOutVol();
}
