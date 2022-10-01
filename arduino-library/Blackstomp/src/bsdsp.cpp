/*!
 *  @file       bsdsp.cpp
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
  
#include "bsdsp.h"
#include "blackstomp.h"
#include <math.h>

float lookupLinear(float x, const float* table)
{
  int index = (int)x;
  float frac = x - (float)index;
  return table[index] + frac * (table[index+1]-table[index]);
}

//######################################################################
// SINE OSCILLATOR
#define MAXPHASE 255.0
oscillator::oscillator()
{
	phase = 0;
	phaseincrement = 0;
	setFrequency(20);
	waveTable = sin_table;
}

void oscillator::setWaveTable(const float* wtable)
{
	if(wtable==NULL)
		waveTable = sin_table;
	else waveTable = wtable;
}

void oscillator::update()
{
  phase = phase + phaseincrement;
  if(phase > MAXPHASE)
    phase = phase-MAXPHASE;
}

void oscillator::setPhase(float p)
{
	phase = p;
}

void oscillator::setFrequency(float freq)
{
  phaseincrement = freq * MAXPHASE /(float)SAMPLE_RATE;
}

float oscillator::getOutput(float phaseOffset)
{
  float p = phase + phaseOffset;
  if(p>MAXPHASE)
    p = p - MAXPHASE;
  return lookupLinear(p,waveTable);
}

float oscillator::getOutput()
{
  return lookupLinear(phase,waveTable);
}

//######################################################################
// BIQUAD FILTER
// Implementation of direct-form-2 biquad iir filter
biquadFilter::biquadFilter(int stageCount)
{
  stages = stageCount;
  states = new biquadState[stages];
  for(int i=0;i<stages;i++)
  {
    states[i].w[0]=0;
    states[i].w[1]=0;
  }
}

biquadFilter::~biquadFilter()
{
  delete[] states;
}

void biquadFilter::setCoef(const float* coef)
{
  for(int i=0;i<stages;i++)
  {
    for(int n=0;n<5;n++)
      states[i].coef[n]=coef[(5*i)+n];
  }
}

void biquadFilter::reset()
{
    for(int i=0;i<stages;i++)
    {
      states[i].w[0]=0;
      states[i].w[1]=0;
    }
}

void biquadFilter::process(const float* in, float* out, int sampleCount)
{
  for(int i=0;i<sampleCount;i++)
  {
    float input = in[i];
    for(int s=0; s<stages; s++) 
    {
      float temp = input + states[s].coef[3] * states[s].w[0] + states[s].coef[4]*states[s].w[1];
      input = temp * states[s].coef[0] + states[s].coef[1] * states[s].w[0] + states[s].coef[2]*states[s].w[1];
      states[s].w[1]=states[s].w[0];
      states[s].w[0]=temp;
    }
    out[i]=input;
  }
}

float biquadFilter::process(float in)
{
	float input = in;
    for(int s=0; s<stages; s++) 
    {
      float temp = input + states[s].coef[3] * states[s].w[0] + states[s].coef[4]*states[s].w[1];
      input = temp * states[s].coef[0] + states[s].coef[1] * states[s].w[0] + states[s].coef[2]*states[s].w[1];
      states[s].w[1]=states[s].w[0];
      states[s].w[0]=temp;
    }
    return input;
}

//######################################################################
// FRACTIONAL DELAY
fractionalDelay::fractionalDelay()
{
	bufflength=0;
	writeIndex=0;
	buffer = NULL;
}

bool fractionalDelay::init(float maxDelayInMs)
{
	if(buffer!=NULL)
		return false;
		
	maxdelay = maxDelayInMs;
	sampleCountPerMs = SAMPLE_RATE/1000;
	bufflength = ceilf(maxDelayInMs * sampleCountPerMs)+1;
	buffer = new float[bufflength];
	
	if(buffer!=NULL)
	{
		for(int i=0;i<bufflength;i++)
			buffer[i]=0;
		return true;
	}
	else return false;
}

fractionalDelay::~fractionalDelay()
{
	if(buffer != NULL)
		delete[] buffer;
}

void fractionalDelay::write(float sample)
{
	writeIndex++;
	if(writeIndex>=bufflength)
		writeIndex=0;
	buffer[writeIndex]=sample;
}

float fractionalDelay::read(float delayInMs)
{
	float indexpos = (float)writeIndex - (sampleCountPerMs * delayInMs);
	if(indexpos < 0)
		indexpos = (float)bufflength + indexpos;
		
	int index0= (int) indexpos;
	float frac = indexpos - (float) index0;
	int index1 = index0 + 1;
	if(index1==bufflength)
		index1=0;
	//linear interpolate at fractional point
	return buffer[index0] + frac * (buffer[index1]-buffer[index0]);
}

//######################################################################
// WAVESHAPER
waveshaper::waveshaper()
{
	//initialize the transfer function table with default function
	for(int i=0;i<256;i++)
	{
		float x = ((float)i-127.5f)/127.5f;
		transferFunctionTable[i]=2.0f/(1+expf(-6.0f*x))-1.0f;
	}
}

float waveshaper::process(float in)
{
	float val = (in+1.0f)*127.5f;
	if(val<0.0f) val = 0.0f;
	if(val>255.0f) val=255.0f;
	return lookupLinear(val,transferFunctionTable);
}

void waveshaper::process(float* in, float* out, int sampleCount)
{
	for(int i=0;i<sampleCount;i++)
	{
		float val = (in[i]+1.0f)*127.5f;
		if(val<0.0f) val = 0.0f;
		if(val>255.0f) val=255.0f;
		out[i]= lookupLinear(val,transferFunctionTable);
	}
}

//######################################################################
// RC HIGH-PASS FILTER
rchipass::rchipass()
{
	vc=0;
	setCutOff(20);
}

void rchipass::setTimeConstant(float val)
{
	tc = val;
}

void rchipass::setCutOff(float val)
{
	tc = 1/(6.283*val);
}

float rchipass::process(float in)
{
	float delta = (in-vc)/(tc*44100);
	vc = vc + delta;
	return in-vc;
}

void rchipass::process(float* in, float* out, int sampleCount)
{
	for(int i=0;i<sampleCount;i++)
	{
		float delta = (in[i]-vc)/(tc*44100);
		vc = vc + delta;
		out[i]=in[i]-vc;
	}
}

//######################################################################
// RC LOW-PASS FILTER
rclopass::rclopass()
{
	vc=0;
	setCutOff(1000);
}

void rclopass::setTimeConstant(float val)
{
	tc = val;
}

void rclopass::setCutOff(float val)
{
	tc = 1/(6.283*val);
}

float rclopass::process(float in)
{
	float delta = (in-vc)/(tc*44100);
	vc = vc + delta;
	return vc;
}

void rclopass::process(float* in, float* out, int sampleCount)
{
	for(int i=0;i<sampleCount;i++)
	{
		float delta = (in[i]-vc)/(tc*44100);
		vc = vc + delta;
		out[i]=vc;
	}
}

//######################################################################
// SIMPLE TONE
simpletone::simpletone()
{
	hiPass.setTimeConstant(0.0001078); //emulate big muff R22k and C4.9nF
	loPass.setTimeConstant(0.00039); //emulate big muff R39k and C10nF
}

void simpletone::setTone(float val)
{
	tone = val;
}

float simpletone::process(float in)
{
	return tone * hiPass.process(in) + (1-tone) * loPass.process(in);
}

void simpletone::process(float* in, float* out, int sampleCount)
{
	for(int i=0;i<sampleCount;i++)
	{
		out[i] = tone * hiPass.process(in[i]) + (1-tone) * loPass.process(in[i]);
	}
}

//######################################################################
// NOISE GATE

noisegate::noisegate()
{
	envelope = 0;
	setThreshold(-70);
	
	lpf = new biquadFilter(2);
	const float co[] = 
	{
	    0.000008577602186511523, 0.000017155204373023046, 0.000008577602186511523, 1.9894924600456316, -0.9895247688375751,// b0, b1, b2, a1, a2
		0.00000762939453125, 0.0000152587890625, 0.00000762939453125, 1.995615255450843, -0.9956476636752434// b0, b1, b2, a1, a2
	};
	lpf->setCoef(co);
}

float noisegate::process(float in)
{
	float level = lpf->process(fabsf(in));
	
	//peak detector
	float delta = level - envelope;
	if(delta > 0)
		envelope += delta;
	else
	{
		envelope += 0.001*delta;
	}
	
	//detecting the gate and expansion area
	if(envelope < lowerTh)
	{
		in = 0;
	}
	else if(envelope < upperTh)
	{
		in = in * (envelope - lowerTh)/(upperTh-lowerTh);
	}
	return in;
}

void noisegate::process(float* in, float* out, int sampleCount)
{
	for(int i=0;i<sampleCount;i++)
	{
		out[i]=process(in[i]);
	}
}

//0 = -70dB, 1 = -10dB
void noisegate::setThreshold(float val)
{
	float dB = -70.0f + 60.0f * val;
	upperTh = powf(10,dB/20.0f);
	lowerTh = upperTh/2.0f;
}
