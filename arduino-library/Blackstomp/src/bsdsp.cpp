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


