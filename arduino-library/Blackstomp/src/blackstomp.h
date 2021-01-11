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

//uncomment the following #define line to use APLL at 6MHZ MCLK
//#define USE_APLL_MCLK_6M

#define SAMPLE_RATE     (44100)

#include "bsdsp.h"
#include "effectmodule.h"
#include "control.h"
#include "ledindicator.h"
#include "btterminal.h"

//BLACKSTOMP'S SYSTEM API

//Blackstomp core setup, 
//should be called inside arduino platform's setup()
void blackstompSetup(effectModule* module); 

//enable BLE (bluetooth low energy) terminal, 
//should be called (if needed) ater blackstompSetup() inside arduino platform's setup()
void enableBleTerminal(void);

//set the output level (analog gain)
void setOutVol(int vol); //vol = 0-31

//get the output level (analog gain)
int  getOutVol(); //return 0-31

//set the output mode for left and right channel,
//if true then the output will contain the mix of left and right signal of the effect outputs
//or of the inputs when the effect is bypassed
bool setOutMix(bool mixedLeft, bool mixedRight);

//set microphone gain (0:0dB,1-7:30dB-48dB)
void setMicGain(int gain);

//get microphone gain (0:0dB,1-7:30dB-48dB)
int getMicGain(); 

//bypassed the analog input to the output, disconnect the digital i/o 
bool analogBypass(bool bypass);  

//bypassed the analog input to the output, disconnect the digital input, preserve the digital output connection
bool analogSoftBypass(bool bypass);  

//Total available Cpu ticks for aduio frame processing
int getTotalCpuTicks(); 

//Number of Cpu ticks used by effect module processing
int getUsedCpuTicks();  

//UsedCpuTicks/TotalCpuTicks
float getCpuUsage();   

//audio frames per second
int getAudioFps();     

//run system monitor on serial port, can be called on arduino setup
//don't call this function when MIDI is implemented
void runSystemMonitor(int baudRate, int updatePeriod);

#endif
