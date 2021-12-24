/*!
 *  @file       blackstomp.h
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
 
#ifndef BLACKSTOMP_H_
#define BLACKSTOMP_H_

#define SAMPLE_RATE     (44100)
#include "bsdsp.h"
#include "effectmodule.h"
#include "control.h"
#include "ledindicator.h"
#include "btterminal.h"
#include "codec.h"

//BLACKSTOMP'S SYSTEM API

//Blackstomp core setup, 
//should be called inside arduino platform's setup()
void blackstompSetup(effectModule* module); 

//Set device type (currently supported types: DT_ESP32_A1S_AC101 (DEFAULT) and DT_ESP32_A1S_ES8388)
void setDeviceType(DEVICE_TYPE dt);

//enable BLE (bluetooth low energy) terminal, 
//should be called (if needed) ater blackstompSetup() inside arduino platform's setup()
void enableBleTerminal(void);

//set the output level (analog gain)
//vol = 0-30 for ES83-version module
//vol = 0-31 for AC101-version module
void setOutVol(int vol); 

//get the output level (analog gain)
int  getOutVol(); 

//set the input gain (analog gain) 
//currently only implemented for ES8388-version module 
//gain: 0..8 for (0,3,6,9,12,15,18,21,24)dB
void setInGain(int gain);

//get the input gain (analog gain) 
//currently only implemented for ES8388-version module 
//gain: 0..8 for (0,3,6,9,12,15,18,21,24)dB
int getInGain();

//optimize the analog to digital conversion range
//currently only implemented for ES8388-version module 
//range: 0, 1, 2, 3, 4, default: 2 (0.25Vrms/707mVpp)
//(1Vrms/2.83Vpp, 0.5Vrms/1.41Vpp, 0.25Vrms/707mVpp, 0.125Vrms/354mVpp, 0.0625Vrms/177mVpp)
void optimizeConversion(int range=2);

//set microphone gain 
//0-7 (0:0dB,1-7:30dB-48dB) for AC101-version module
//0-8 (12,15,18,21,24,27,30,33,36)dB for ES8388-version module 
//(ES8388 mic gain is formulated from PGA gain + ALC gain + Software Correction)
void setMicGain(int gain);
int getMicGain(); 

//mic's noise gate 
//gate: 0-32 (n.g.off, -76.5dB, -75.0dB,...., -30.0dB for ES8388-version module)
//currently only implemented for ES8388-version module 
void setMicNoiseGate(int gate);
int getMicNoiseGate();

//bypassed the analog input to the output, disconnect the digital i/o 
bool analogBypass(bool bypass, BYPASS_MODE bm=BM_LR);  

//bypassed the analog input to the output, disconnect the digital input, preserve the digital output connection
bool analogSoftBypass(bool bypass, BYPASS_MODE bm=BM_LR);

//Total available Cpu ticks for aduio frame processing
int getTotalCpuTicks(); 

//Number of Cpu ticks used by effect module processing
int getUsedCpuTicks();  

//UsedCpuTicks/TotalCpuTicks
float getCpuUsage();   

//audio frames per second
int getAudioFps();     

//run system monitor on serial port, should be called on arduino setup when needed
//don't call this function when runScope function has been called
//don't call this function when MIDI is implemented
void runSystemMonitor(int baudRate=115200, int updatePeriod=1500);

//run 2-channel simple scope, should be called on arduino setup when needed
//don't call this function when runSystemMonitor function has been called
//don't call this function when MIDI is implemented
void runScope(int baudRate=1000000, int sampleLength=441, int triggerChannel=0, float triggerLevel=0, bool risingTrigger=true);

//probe a signal to be displayed on Arduino IDE's serial plotter
//channel: 0-1
void scopeProbe(float sample, int channel);

//set debug string (to be shown in the system monitor when it runs)
void setDebugStr(const char* str);

//set debug variables (to be shown in the system monitor when it runs)
void setDebugVars(float val1, float val2=0, float val3=0, float val4=0);

#endif
