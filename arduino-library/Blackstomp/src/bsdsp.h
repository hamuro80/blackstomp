/*!
 *  @file       bsdsp.h
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
  
#ifndef BSDSP_H_
#define BSDSP_H_

#include "dsptable.h"

// x = ranges from 0.0 to 255.0; table size is 256 (table[0] to table[255])
float lookupLinear(float x, const float* table);

class biquadFilter
{
  private:
    int stages;
    void* states;
  public:
  float process(float in);
  void process(const float* in, float* out, int sampleCount);
  void setCoef(const float* coef);  //coef[] = {coef stage0, coeff stage1,..} = {b0,b1,b2,a1,a2,b0,b1,b2,a1,a2,..}
  void reset();

  biquadFilter(int stageCount);
  ~biquadFilter();
};

class fractionalDelay
{
	private:
	int bufflength;
	int writeIndex;
	float* buffer;
	float sampleCountPerMs;
	float maxdelay;
	
	public:
	fractionalDelay(); 
	~fractionalDelay();
	bool init(float maxDelayInMs);//max delay in milliseconds
	void write(float sample);
	float read(float delayInMs);
};

class oscillator
{
	private:
	float phaseincrement;
	float phase;
	const float* waveTable;
	
	public:
	oscillator();
	void setFrequency(float freq);
	void setPhase(float p);  //(0.00 <= p <= 255.0)
	void setWaveTable(const float* wtable);
  
  //update the internal phase to the current sampling period
	void update(); 

  //phase offset = 0 .. 255.0
	float getOutput(float phaseOffset);
	float getOutput();
};

class waveShaper
{
	public:
		//constructor
		waveShaper();
		float transferFunctionTable[256];
		//block processing mode
		void process(float* in, float* out, int sampleCount);
		//sample processing mode
		float process(float in);
};

class rcHiPass
{
	private:
		float vc;	//capacitor voltage
		float tc;	//time constant
	public:
	rcHiPass();
	void setCutOff(float val);
	void setTimeConstant(float val);
	float process(float in);
	void process(float* in, float* out, int sampleCount);
};

class rcLoPass
{
	private:
		float vc;	//capacitor voltage
		float tc;	//time constant
	public:
	rcLoPass();
	void setCutOff(float val);
	void setTimeConstant(float val);
	float process(float in);
	void process(float* in, float* out, int sampleCount);
};

class simpleTone
{
	private:
	float tone;
	
	public:
	simpleTone();
	rcLoPass loPass;
	rcHiPass hiPass;
	float process(float in);
	void process(float* in, float* out, int sampleCount);
	void setTone(float val);	//0.0-1.0
};

class noiseGate
{
	private:
	biquadFilter* lpf;
	float upperTh;
	float lowerTh;
	float envelope;
	
	public:
	noiseGate();
	float process(float in);
	void process(float* in, float* out, int sampleCount);
	void setThreshold(float val); //0 = -70dB, 1 = -10dB
};

#endif
