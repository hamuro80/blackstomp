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

#include "AC101.h"
#include <Wire.h>

#define AC101_ADDR			0x1A
#define CHIP_AUDIO_RS		0x00
#define PLL_CTRL1			0x01
#define PLL_CTRL2			0x02
#define SYSCLK_CTRL			0x03
#define MOD_CLK_ENA			0x04
#define MOD_RST_CTRL		0x05
#define I2S_SR_CTRL			0x06
#define I2S1LCK_CTRL		0x10
#define I2S1_SDOUT_CTRL		0x11
#define I2S1_SDIN_CTRL		0x12
#define I2S1_MXR_SRC		0x13
#define I2S1_VOL_CTRL1		0x14
#define I2S1_VOL_CTRL2		0x15
#define I2S1_VOL_CTRL3		0x16
#define I2S1_VOL_CTRL4		0x17
#define I2S1_MXR_GAIN		0x18
#define ADC_DIG_CTRL		0x40
#define ADC_VOL_CTRL		0x41
#define HMIC_CTRL1			0x44
#define HMIC_CTRL2			0x45
#define HMIC_STATUS			0x46
#define DAC_DIG_CTRL		0x48
#define DAC_VOL_CTRL		0x49
#define DAC_MXR_SRC			0x4C
#define DAC_MXR_GAIN		0x4D
#define ADC_APC_CTRL		0x50
#define ADC_SRC				0x51
#define ADC_SRCBST_CTRL		0x52
#define OMIXER_DACA_CTRL	0x53
#define OMIXER_SR			0x54
#define OMIXER_BST1_CTRL	0x55
#define HPOUT_CTRL			0x56
#define SPKOUT_CTRL			0x58
#define AC_DAC_DAPCTRL		0xA0
#define AC_DAC_DAPHHPFC 	0xA1
#define AC_DAC_DAPLHPFC 	0xA2
#define AC_DAC_DAPLHAVC 	0xA3
#define AC_DAC_DAPLLAVC 	0xA4
#define AC_DAC_DAPRHAVC 	0xA5
#define AC_DAC_DAPRLAVC 	0xA6
#define AC_DAC_DAPHGDEC 	0xA7
#define AC_DAC_DAPLGDEC 	0xA8
#define AC_DAC_DAPHGATC 	0xA9
#define AC_DAC_DAPLGATC 	0xAA
#define AC_DAC_DAPHETHD 	0xAB
#define AC_DAC_DAPLETHD 	0xAC
#define AC_DAC_DAPHGKPA 	0xAD
#define AC_DAC_DAPLGKPA 	0xAE
#define AC_DAC_DAPHGOPA 	0xAF
#define AC_DAC_DAPLGOPA 	0xB0
#define AC_DAC_DAPOPT   	0xB1
#define DAC_DAP_ENA     	0xB5
#define ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))

const uint8_t regs[] = {
	 CHIP_AUDIO_RS		,
	 PLL_CTRL1			,
	 PLL_CTRL2			,
	 SYSCLK_CTRL		,
	 MOD_CLK_ENA		,
	 MOD_RST_CTRL		,
	 I2S_SR_CTRL		,
	 I2S1LCK_CTRL		,
	 I2S1_SDOUT_CTRL	,
	 I2S1_SDIN_CTRL		,
	 I2S1_MXR_SRC		,
	 I2S1_VOL_CTRL1		,
	 I2S1_VOL_CTRL2		,
	 I2S1_VOL_CTRL3		,
	 I2S1_VOL_CTRL4		,
	 I2S1_MXR_GAIN		,
	 ADC_DIG_CTRL		,
	 ADC_VOL_CTRL		,
	 HMIC_CTRL1			,
	 HMIC_CTRL2			,
	 HMIC_STATUS		,
	 DAC_DIG_CTRL		,
	 DAC_VOL_CTRL		,
	 DAC_MXR_SRC		,
	 DAC_MXR_GAIN		,
	 ADC_APC_CTRL		,
	 ADC_SRC			,
	 ADC_SRCBST_CTRL	,
	 OMIXER_DACA_CTRL	,
	 OMIXER_SR			,
	 OMIXER_BST1_CTRL	,
	 HPOUT_CTRL			,
	 SPKOUT_CTRL		,
	 AC_DAC_DAPCTRL		,
	 AC_DAC_DAPHHPFC 	,
	 AC_DAC_DAPLHPFC 	,
	 AC_DAC_DAPLHAVC 	,
	 AC_DAC_DAPLLAVC 	,
	 AC_DAC_DAPRHAVC 	,
	 AC_DAC_DAPRLAVC 	,
	 AC_DAC_DAPHGDEC 	,
	 AC_DAC_DAPLGDEC 	,
	 AC_DAC_DAPHGATC 	,
	 AC_DAC_DAPLGATC 	,
	 AC_DAC_DAPHETHD 	,
	 AC_DAC_DAPLETHD 	,
	 AC_DAC_DAPHGKPA 	,
	 AC_DAC_DAPLGKPA 	,
	 AC_DAC_DAPHGOPA 	,
	 AC_DAC_DAPLGOPA 	,
	 AC_DAC_DAPOPT   	,
	 DAC_DAP_ENA
};

#define RIGHT_MIC_ENABIT  13
#define RIGHT_LINE_ENABIT 10
#define LEFT_MIC_ENABIT   5
#define LEFT_LINE_ENABIT  3

bool AC101::WriteReg(uint8_t reg, uint16_t val)
{
	Wire.beginTransmission(AC101_ADDR);
	Wire.write(reg);
	Wire.write(uint8_t((val >> 8) & 0xff));
	Wire.write(uint8_t(val & 0xff));
	return 0 == Wire.endTransmission(true);
}

uint16_t AC101::ReadReg(uint8_t reg)
{
	Wire.beginTransmission(AC101_ADDR);
	Wire.write(reg);
	Wire.endTransmission(false);

	uint16_t val = 0u;
	if (2 == Wire.requestFrom(uint16_t(AC101_ADDR), uint8_t(2), true))
	{
		val = uint16_t(Wire.read() << 8) + uint16_t(Wire.read());
	}
	Wire.endTransmission(false);

	return val;
}

AC101::AC101()
{
	
}

bool AC101::setup(int sda, int scl, uint32_t frequency)
{
	bool ok = Wire.begin(sda, scl, frequency);

	ok &= WriteReg(CHIP_AUDIO_RS, 0x123);
	delay(100);
	ok &= 0x0101 == ReadReg(CHIP_AUDIO_RS);
	ok &= WriteReg(SPKOUT_CTRL, 0xe880);

	// Enable the PLL from 256*44.1KHz MCLK source
	ok &= WriteReg(PLL_CTRL1, 0x014f);
	ok &= WriteReg(PLL_CTRL2, 0x8600);

	// Clocking system
	ok &= WriteReg(SYSCLK_CTRL, 0x8b08);
	ok &= WriteReg(MOD_CLK_ENA, 0x800c);
	ok &= WriteReg(MOD_RST_CTRL, 0x800c);

	// Set default at I2S, 44.1KHz, 24bit
	ok &= SetI2sSampleRate(SAMPLE_RATE_44100);
	ok &= SetI2sClock(BCLK_DIV_1, false, LRCK_DIV_64, false);
	ok &= SetI2sMode(MODE_SLAVE);
	ok &= SetI2sWordSize(WORD_SIZE_24_BITS);
	ok &= SetI2sFormat(DATA_FORMAT_I2S);

	// AIF config
	ok &= WriteReg(I2S1_SDOUT_CTRL, 0xc000);
	ok &= WriteReg(I2S1_SDIN_CTRL, 0xc000);
	ok &= WriteReg(I2S1_MXR_SRC, 0x2200);

	ok &= WriteReg(ADC_SRCBST_CTRL, 0xccc4); //enable mic1 and mic2 with boost
	ok &= WriteReg(ADC_SRC, 0x0);	//mute all source
	ok &= WriteReg(ADC_DIG_CTRL, 0x8000); //enable adc
	ok &= WriteReg(ADC_APC_CTRL, 0xbbc6); //enable adc left, adc right, mic bias

	// Path Configuration
	ok &= WriteReg(DAC_MXR_SRC, 0xcc00);
	ok &= WriteReg(DAC_DIG_CTRL, 0x8000); //enable dac
	ok &= WriteReg(OMIXER_SR, 0x0102); //select only from dac
	ok &= WriteReg(OMIXER_DACA_CTRL, 0xf080); //enabl ldac, rdac, lmixer, rmixer, 
	
	//ok &= WriteReg(ADC_SRC, 0x4408); //select line-l and line-r input
	//ok &= WriteReg(ADC_SRC, 0x2020); //select mic1 and mic2 input
	
	ok &= WriteReg(ADC_DIG_CTRL, 0x8000); //enable adc
	ok &= WriteReg(MOD_CLK_ENA,  0x800c);
	ok &= WriteReg(MOD_RST_CTRL, 0x800c);

	// Enable Headphone output
	ok &= WriteReg(OMIXER_DACA_CTRL, 0xff80);
	ok &= WriteReg(HPOUT_CTRL, 0xc3c1);	
	ok &= WriteReg(HPOUT_CTRL, 0xcb00);
	delay(100);
	ok &= WriteReg(HPOUT_CTRL, 0xfbc0);
	ok &= SetVolHeadphone(60);
	// Enable Speaker output
	ok &= WriteReg(SPKOUT_CTRL, 0xeabd);
	delay(10);
	ok &= SetVolSpeaker(60);

	return ok;
}

uint8_t AC101::GetVolSpeaker()
{
	// Times 2 to match the headphone volume
	return (ReadReg(SPKOUT_CTRL) & 31) * 2;
}

bool AC101::RightMicEnable(bool enable)
{
  uint16_t val = ReadReg(ADC_SRC);
  val &=  ~( (uint16_t)1 << RIGHT_MIC_ENABIT);
  val |=  (uint16_t)enable << RIGHT_MIC_ENABIT;
  return WriteReg(ADC_SRC, val);
}

bool AC101::RightLineEnable(bool enable)
{
  uint16_t val = ReadReg(ADC_SRC);
  val &=  ~( (uint16_t)1 << RIGHT_LINE_ENABIT);
  val |=  (uint16_t)enable << RIGHT_LINE_ENABIT;
  return WriteReg(ADC_SRC, val);
}

bool AC101::LeftMicEnable(bool enable)
{
  uint16_t val = ReadReg(ADC_SRC);
  val &=  ~( (uint16_t)1 << LEFT_MIC_ENABIT);
  val |=  (uint16_t)enable << LEFT_MIC_ENABIT;
  return WriteReg(ADC_SRC, val);
}

bool AC101::LeftLineEnable(bool enable)
{
  uint16_t val = ReadReg(ADC_SRC);
  val &=  ~( (uint16_t)1 << LEFT_LINE_ENABIT);
  val |=  (uint16_t)enable << LEFT_LINE_ENABIT;
  return WriteReg(ADC_SRC, val);
}

bool AC101::SetVolSpeaker(uint8_t volume)
{
	// Divide by 2 to match the headphone volume
	volume /= 2;
	if (volume > 31) volume = 31;

	uint16_t val = ReadReg(SPKOUT_CTRL);
	val &= ~31;
	val |= volume;
	return WriteReg(SPKOUT_CTRL, val);
}

uint8_t AC101::GetVolHeadphone()
{
	return (ReadReg(HPOUT_CTRL) >> 4) & 63;
}

bool AC101::SetVolHeadphone(uint8_t volume)
{
	if (volume > 63) volume = 63;

	uint16_t val = ReadReg(HPOUT_CTRL);
	val &= ~63 << 4;
	val |= volume << 4;
	return WriteReg(HPOUT_CTRL, val);
}

bool AC101::SetI2sSampleRate(uint16_t rate)
{
	return WriteReg(I2S_SR_CTRL, rate);
}

bool AC101::SetI2sMode(uint16_t mode)
{
	uint16_t val = ReadReg(I2S1LCK_CTRL);
	val &= ~0x8000;
	val |= uint16_t(mode) << 15;
	return WriteReg(I2S1LCK_CTRL, val);
}

bool AC101::SetI2sWordSize(uint16_t size)
{
	uint16_t val = ReadReg(I2S1LCK_CTRL);
	val &= ~0x0030;
	val |= uint16_t(size) << 4;
	return WriteReg(I2S1LCK_CTRL, val);
}

bool AC101::SetI2sFormat(uint16_t format)
{
	uint16_t val = ReadReg(I2S1LCK_CTRL);
	val &= ~0x000C;
	val |= uint16_t(format) << 2;
	return WriteReg(I2S1LCK_CTRL, val);
}

bool AC101::SetI2sClock(uint16_t bitClockDiv, uint16_t bitClockInv, uint16_t lrClockDiv, uint16_t lrClockInv)
{
	uint16_t val = ReadReg(I2S1LCK_CTRL);
	val &= ~0x7FC0;
	val |= (bitClockInv ? 1 : 0) << 14;
	val |= bitClockDiv << 9;
	val |= (lrClockInv ? 1 : 0) << 13;
	val |= lrClockDiv << 6;
	return WriteReg(I2S1LCK_CTRL, val);
}
