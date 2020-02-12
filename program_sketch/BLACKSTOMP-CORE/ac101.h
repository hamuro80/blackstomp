/*
  * ESP32-A1S AC101 Codec driver library for Arduino
  * Author: HAMURO
  * COPYRIGHT(c) 2020 DEEPTRONIC.COM      
  * 
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of DEEPTRONIC.COM nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#ifndef AC101_H
#define AC101_H

#include <inttypes.h>

enum 
{
		SAMPLE_RATE_8000	= 0x0000,
		SAMPLE_RATE_11052	= 0x1000,
		SAMPLE_RATE_12000	= 0x2000,
		SAMPLE_RATE_16000	= 0x3000,
		SAMPLE_RATE_22050	= 0x4000,
		SAMPLE_RATE_24000	= 0x5000,
		SAMPLE_RATE_32000	= 0x6000,
		SAMPLE_RATE_44100	= 0x7000,
		SAMPLE_RATE_48000	= 0x8000,
		SAMPLE_RATE_96000	= 0x9000,
		SAMPLE_RATE_192000	= 0xa000,
};

enum { MODE_MASTER = 0x00,	MODE_SLAVE = 0x01 };

enum 
{
	WORD_SIZE_8_BITS	= 0x00,
	WORD_SIZE_16_BITS	= 0x01,
	WORD_SIZE_20_BITS	= 0x02,
	WORD_SIZE_24_BITS	= 0x03,
};

enum 
{
	DATA_FORMAT_I2S		= 0x00,
	DATA_FORMAT_LEFT	= 0x01,
	DATA_FORMAT_RIGHT	= 0x02,
	DATA_FORMAT_DSP		= 0x03,
};

enum 
{
	BCLK_DIV_1			= 0x0,
	BCLK_DIV_2			= 0x1,
	BCLK_DIV_4			= 0x2,
	BCLK_DIV_6			= 0x3,
	BCLK_DIV_8			= 0x4,
	BCLK_DIV_12			= 0x5,
	BCLK_DIV_16			= 0x6,
	BCLK_DIV_24			= 0x7,
	BCLK_DIV_32			= 0x8,
	BCLK_DIV_48			= 0x9,
	BCLK_DIV_64			= 0xa,
	BCLK_DIV_96			= 0xb,
	BCLK_DIV_128		= 0xc,
	BCLK_DIV_192		= 0xd,
};

enum 
{
	LRCK_DIV_16			= 0x0,
	LRCK_DIV_32			= 0x1,
	LRCK_DIV_64			= 0x2,
	LRCK_DIV_128		= 0x3,
	LRCK_DIV_256		= 0x4,
};

class AC101
{
public:
	// Constructor.
  	AC101();
	bool setup(int sda = -1, int scl = -1, uint32_t frequency = 400000);
	uint8_t GetVolSpeaker(); //0-63
	bool SetVolSpeaker(uint8_t volume); //0-63
	uint8_t GetVolHeadphone(); //0-63
	bool SetVolHeadphone(uint8_t volume); //0-63
  
	//leftchannel input selector methods
	bool LeftMic1(bool select);		//left channel mic1 select
	bool LeftLineDiff(bool select);	//left channel line difference (line Left- Line Right)
	bool LeftLineLeft(bool select);	//left channel line (L)
		
	//rightchannel input selector methods
	bool RightMic1(bool select);		//right channel mic1 select
	bool RightLineDiff(bool select);	//right channel line difference (line Left- Line Right)
	bool RightLineRight(bool select);	//right channel line (R)

	//i2s configuration
	bool SetI2sSampleRate(uint16_t rate);
	bool SetI2sMode(uint16_t mode);
	bool SetI2sWordSize(uint16_t size);
	bool SetI2sFormat(uint16_t format);
	bool SetI2sClock(uint16_t bitClockDiv, uint16_t bitClockInv, uint16_t lrClockDiv, uint16_t lrClockInv);

	
protected:
	bool WriteReg(uint8_t reg, uint16_t val);
	uint16_t ReadReg(uint8_t reg);
};

#endif
