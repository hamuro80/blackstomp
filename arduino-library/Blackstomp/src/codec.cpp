#include "codec.h"
#include <Wire.h>

//ES8388 REGISTER
////////////////////////////////////////////////////////////////////////
#define ES8388_CONTROL1         0x00
#define ES8388_CONTROL2         0x01

#define ES8388_CHIPPOWER        0x02

#define ES8388_ADCPOWER         0x03
#define ES8388_DACPOWER         0x04

#define ES8388_CHIPLOPOW1       0x05
#define ES8388_CHIPLOPOW2       0x06

#define ES8388_ANAVOLMANAG      0x07

#define ES8388_MASTERMODE       0x08

/* ADC */
#define ES8388_ADCCONTROL1      0x09
#define ES8388_ADCCONTROL2      0x0a
#define ES8388_ADCCONTROL3      0x0b
#define ES8388_ADCCONTROL4      0x0c
#define ES8388_ADCCONTROL5      0x0d
#define ES8388_ADCCONTROL6      0x0e
#define ES8388_ADCCONTROL7      0x0f
#define ES8388_ADCCONTROL8      0x10
#define ES8388_ADCCONTROL9      0x11
#define ES8388_ADCCONTROL10     0x12
#define ES8388_ADCCONTROL11     0x13
#define ES8388_ADCCONTROL12     0x14
#define ES8388_ADCCONTROL13     0x15
#define ES8388_ADCCONTROL14     0x16
/* DAC */
#define ES8388_DACCONTROL1      0x17
#define ES8388_DACCONTROL2      0x18
#define ES8388_DACCONTROL3      0x19
#define ES8388_DACCONTROL4      0x1a
#define ES8388_DACCONTROL5      0x1b
#define ES8388_DACCONTROL6      0x1c
#define ES8388_DACCONTROL7      0x1d
#define ES8388_DACCONTROL8      0x1e
#define ES8388_DACCONTROL9      0x1f
#define ES8388_DACCONTROL10     0x20
#define ES8388_DACCONTROL11     0x21
#define ES8388_DACCONTROL12     0x22
#define ES8388_DACCONTROL13     0x23
#define ES8388_DACCONTROL14     0x24
#define ES8388_DACCONTROL15     0x25
#define ES8388_DACCONTROL16     0x26
#define ES8388_DACCONTROL17     0x27
#define ES8388_DACCONTROL18     0x28
#define ES8388_DACCONTROL19     0x29
#define ES8388_DACCONTROL20     0x2a
#define ES8388_DACCONTROL21     0x2b
#define ES8388_DACCONTROL22     0x2c
#define ES8388_DACCONTROL23     0x2d
#define ES8388_DACCONTROL24     0x2e
#define ES8388_DACCONTROL25     0x2f
#define ES8388_DACCONTROL26     0x30
#define ES8388_DACCONTROL27     0x31
#define ES8388_DACCONTROL28     0x32
#define ES8388_DACCONTROL29     0x33
#define ES8388_DACCONTROL30     0x34

//AC101 REGISTERS
////////////////////////////////////////////////////////////////////////
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
#define LEFT_MIC1_ENABIT		6
#define LEFT_LINELEFT_ENABIT	3
#define LEFT_LINEDIFF_ENABIT	4
#define RIGHT_MIC1_ENABIT		13
#define RIGHT_LINERIGHT_ENABIT	10
#define RIGHT_LINEDIFF_ENABIT	11

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

//TwoWire object pointer
static TwoWire * _codecWire = NULL;

//FUNCTION

//Initialize the I2C bus at specific pins and clock frequency
bool codecBusInit(int sdaPin, int sclPin, int frequency)
{
	_codecWire = new TwoWire(0);
	if(_codecWire->begin(sdaPin, sclPin, frequency)==0)
		return false;
	else return true;
}

///////////////////////////////////////////////////////////////////////
//AC101 codec class
bool AC101Codec::writeReg(uint8_t reg, uint16_t val)
{
	if(_codecWire == NULL)
		return false;
	_codecWire->beginTransmission(i2cAddress);
	_codecWire->write(reg);
	_codecWire->write(uint8_t((val >> 8) & 0xff));
	_codecWire->write(uint8_t(val & 0xff));
	return (0 == _codecWire->endTransmission(true));
}
uint16_t AC101Codec::readReg(uint8_t reg)
{
	_codecWire->beginTransmission(i2cAddress);
	_codecWire->write(reg);
	_codecWire->endTransmission(false);
	uint16_t val = 0u;
	if (2 == _codecWire->requestFrom(uint16_t(i2cAddress), uint8_t(2), true))
	{
		val = uint16_t(_codecWire->read() << 8) + uint16_t(_codecWire->read());
	}
	_codecWire->endTransmission(false);
	return val;
}; 

bool AC101Codec::setI2sClock(uint16_t bitClockDiv, uint16_t bitClockInv, uint16_t lrClockDiv, uint16_t lrClockInv)
{
	uint16_t val = readReg(I2S1LCK_CTRL);
	val &= ~0x7FC0;
	val |= (bitClockInv ? 1 : 0) << 14;
	val |= bitClockDiv << 9;
	val |= (lrClockInv ? 1 : 0) << 13;
	val |= lrClockDiv << 6;
	return writeReg(I2S1LCK_CTRL, val);
}

bool AC101Codec::setI2sFormat(uint16_t format)
{
	uint16_t val = readReg(I2S1LCK_CTRL);
	val &= ~0x000C;
	val |= uint16_t(format) << 2;
	return writeReg(I2S1LCK_CTRL, val);
}

bool AC101Codec::setI2sMode(uint16_t mode)
{
	uint16_t val = readReg(I2S1LCK_CTRL);
	val &= ~0x8000;
	val |= uint16_t(mode) << 15;
	return writeReg(I2S1LCK_CTRL, val);
}

bool AC101Codec::setI2sSampleRate(uint16_t rate)
{
	return writeReg(I2S_SR_CTRL, rate);
}

bool AC101Codec::setI2sWordSize(uint16_t size)
{
	uint16_t val = readReg(I2S1LCK_CTRL);
	val &= ~0x0030;
	val |= uint16_t(size) << 4;
	return writeReg(I2S1LCK_CTRL, val);
}

bool AC101Codec::omixerLeftLineLeft(bool select)
{
  uint16_t val = readReg(OMIXER_SR);
  if(select)
    val |= (uint16_t)1<<3;
  else val &= ~((uint16_t)1<<3);
  return writeReg(OMIXER_SR, val);
}

bool AC101Codec::omixerLeftDacLeft(bool select)
{
  uint16_t val = readReg(OMIXER_SR);
  if(select)
    val |= (uint16_t)1<<1;
  else val &= ~((uint16_t)1<<1);
  return writeReg(OMIXER_SR, val);
}

bool AC101Codec::omixerLeftMic1(bool select)
{
  uint16_t val = readReg(OMIXER_SR);
  if(select)
    val |= (uint16_t)1<<6;
  else val &= ~((uint16_t)1<<6);
  return writeReg(OMIXER_SR, val);
}

bool AC101Codec::omixerRightLineRight(bool select)
{
  uint16_t val = readReg(OMIXER_SR);
  if(select)
    val |= (uint16_t)1<<10;
  else val &= ~((uint16_t)1<<10);
  return writeReg(OMIXER_SR, val);
}

bool AC101Codec::omixerRightDacRight(bool select)
{
  uint16_t val = readReg(OMIXER_SR);
  if(select)
    val |= (uint16_t)1<<8;
  else val &= ~((uint16_t)1<<8);
  return writeReg(OMIXER_SR, val);
}

bool AC101Codec::omixerRightMic1(bool select)
{
  uint16_t val = readReg(OMIXER_SR);
  if(select)
    val |= (uint16_t)1<<13;
  else val &= ~((uint16_t)1<<13);
  return writeReg(OMIXER_SR, val);
}

bool AC101Codec::leftMic1(bool select)
{
	uint16_t val = readReg(ADC_SRC);
	if(select)
		val |= (uint16_t)1<<LEFT_MIC1_ENABIT;
	else val &= ~((uint16_t)1<<LEFT_MIC1_ENABIT);
	return writeReg(ADC_SRC, val);
}
 
bool AC101Codec::rightMic1(bool select)
{
	uint16_t val = readReg(ADC_SRC);
	if(select)
		val |= (uint16_t)1<<RIGHT_MIC1_ENABIT;
	else val &= ~((uint16_t)1<<RIGHT_MIC1_ENABIT);
	return writeReg(ADC_SRC, val);
}

bool AC101Codec::leftLineLeft(bool select)
{
	uint16_t val = readReg(ADC_SRC);
	if(select)
		val |= (uint16_t)1<<LEFT_LINELEFT_ENABIT;
	else val &= ~((uint16_t)1<<LEFT_LINELEFT_ENABIT);
	return writeReg(ADC_SRC, val);
}

bool AC101Codec::rightLineRight(bool select)
{
	uint16_t val = readReg(ADC_SRC);
	if(select)
		val |= (uint16_t)1<<RIGHT_LINERIGHT_ENABIT;
	else val &= ~((uint16_t)1<<RIGHT_LINERIGHT_ENABIT);
	return writeReg(ADC_SRC, val);
}

bool AC101Codec::leftLineDiff(bool select)
{
	uint16_t val = readReg(ADC_SRC);
	if(select)
		val |= (uint16_t)1<<LEFT_LINEDIFF_ENABIT;
	else val &= ~((uint16_t)1<<LEFT_LINEDIFF_ENABIT);
	return writeReg(ADC_SRC, val);
}

bool AC101Codec::rightLineDiff(bool select)
{
	uint16_t val = readReg(ADC_SRC);
	if(select)
		val |= (uint16_t)1<<RIGHT_LINEDIFF_ENABIT;
	else val &= ~((uint16_t)1<<RIGHT_LINEDIFF_ENABIT);
	return writeReg(ADC_SRC, val);
}


bool AC101Codec::init(int address)
{
	outCorrectionGain = 1;
	i2cAddress = address;
	bool ok = true;
	ok &= writeReg(CHIP_AUDIO_RS, 0x123);
	delay(100);
	ok &= 0x0101 == readReg(CHIP_AUDIO_RS);
	ok &= writeReg(SPKOUT_CTRL, 0xe880);

	// Enable the PLL from BCLK source at 44100 sample/s (BCLK = 64.fs = 2.8224M)
	// (FOUT=22.5792M, FIN=2.8224M, M=1, N=24, K=1, see AC101 codec datasheet)
	// PLL_CTRL1 = 0000 0001 0100 1111 (0x14f)
	// PLL_CTRL2 = 1000 0001 1000 0000 (0x8180)
	ok &= writeReg(PLL_CTRL1, 0x14f);
	ok &= writeReg(PLL_CTRL2, 0x8180);
	ok &= writeReg(SYSCLK_CTRL, 0xab08); //set the source from BCLK

	ok &= writeReg(MOD_CLK_ENA, 0x800c);
	ok &= writeReg(MOD_RST_CTRL, 0x800c);
    ok &= setI2sSampleRate(SAMPLE_RATE_44100);

	ok &= setI2sClock(BCLK_DIV_8, false, LRCK_DIV_64, true);
	ok &= setI2sMode(MODE_SLAVE);
	ok &= setI2sWordSize(WORD_SIZE_24_BITS);
	ok &= setI2sFormat(DATA_FORMAT_I2S);

	// AIF config
	ok &= writeReg(I2S1_SDOUT_CTRL, 0xc000);
	ok &= writeReg(I2S1_SDIN_CTRL, 0xc000);
	ok &= writeReg(I2S1_MXR_SRC, 0x2200);

	ok &= writeReg(ADC_SRCBST_CTRL, 0xccc4); //enable mic1 and mic2 with boost
	ok &= writeReg(ADC_SRC, 0x0);	//mute all source
	ok &= writeReg(ADC_DIG_CTRL, 0x8000); //enable adc
	ok &= writeReg(ADC_APC_CTRL, 0xbbc6); //enable adc left, adc right, mic bias

	// Path Configuration
	ok &= writeReg(DAC_MXR_SRC, 0xcc00);
	ok &= writeReg(DAC_DIG_CTRL, 0x8000); //enable dac
	ok &= writeReg(OMIXER_SR, 0x0102); //select only from dac
	ok &= writeReg(OMIXER_DACA_CTRL, 0xf080); //enabl ldac, rdac, lmixer, rmixer, 
	
	ok &= writeReg(ADC_DIG_CTRL, 0x8000); //enable adc
	ok &= writeReg(MOD_CLK_ENA,  0x800c);
	ok &= writeReg(MOD_RST_CTRL, 0x800c);

	// Eenable Output mixer and DAC
	ok &= writeReg(OMIXER_DACA_CTRL, 0xff80);
 
	// Enable Speaker output
	ok &= writeReg(SPKOUT_CTRL, 0xeabd);
	delay(10);
	// set the volume at 0dB
	ok &= setOutVol(31);
	
	//setup the input selection
	if(getInputMode() == 0) //mode IM_LR
    {
		leftLineLeft(true);
		rightLineRight(true);
    }
    else //mode IM_LMIC
    {
		leftLineLeft(true);
		rightMic1(true);
    }

	return ok;
}
	
//get and set the output level (analog gain)
//vol = 0-31
bool AC101Codec::setOutVol(int vol)
{
	if (vol > 31) vol = 31;
	uint16_t val = readReg(SPKOUT_CTRL);
	val &= ~31;
	val |= vol;
	return writeReg(SPKOUT_CTRL, val);
}

int AC101Codec::getOutVol()
{
	return (readReg(SPKOUT_CTRL) & 31);
}
	
//get and set microphone gain (0:0dB,1-7:30dB-48dB)
uint8_t AC101Codec::getMicGain()
{
	uint16_t val = readReg(ADC_SRCBST_CTRL);
	return (uint8_t)(val >> 12) & 7;
}

bool AC101Codec::setMicGain(uint8_t gain)
{
	if(gain > 7) gain = 7;
	uint16_t val = readReg(ADC_SRCBST_CTRL);
	val &=  ~(7 << 12);
	val |= gain << 12;
	return writeReg(ADC_SRCBST_CTRL, val);
}
	
//bypassed the analog input to the output, disconnect the digital i/o 
bool AC101Codec::analogBypass(bool bypass, BYPASS_MODE bm)
{
	//connect or disconnect the DAC output to output mixer
    omixerRightDacRight(!bypass);
    omixerLeftDacLeft(!bypass);

    //connect or disconnect the ADC input
    if(getInputMode() == 0) //mode IM_LR
    {
		if((bm==BM_LR)||(bm==BM_L))
			leftLineLeft(!bypass);
		if((bm==BM_LR)||(bm==BM_R))
			rightLineRight(!bypass);
    }
    else //mode IM_LMIC
    {
		if((bm==BM_LR)||(bm==BM_L))
			leftLineLeft(!bypass);
		if((bm==BM_LR)||(bm==BM_R))
			rightMic1(!bypass);
    }

    //connect/disconnect the line/mic input to output mixer
    if(getInputMode() == 0) //mode = IM_LR
    {      
		if((bm==BM_LR)||(bm==BM_L))
			omixerLeftLineLeft(bypass);
		if((bm==BM_LR)||(bm==BM_R))
			omixerRightLineRight(bypass);
    }
    else //inputMode = IM_LMIC
    {
		if((bm==BM_LR)||(bm==BM_L))
			omixerLeftLineLeft(bypass);
		if((bm==BM_LR)||(bm==BM_R))
			omixerRightMic1(bypass);
    }
    return true;
}

//bypassed the analog input to the output, disconnect the digital input, preserve the digital output connection
bool AC101Codec::analogSoftBypass(bool bypass, BYPASS_MODE bm)
{
	//always connect the DAC output to output mixer
    omixerRightDacRight(true);
    omixerLeftDacLeft(true);

    //connect or disconnect the ADC input
    if(getInputMode() == 0) //input mode = IM_LR
    {
		if((bm==BM_LR)||(bm==BM_L))
			leftLineLeft(!bypass);
		if((bm==BM_LR)||(bm==BM_R))
			rightLineRight(!bypass);
    }
    else //input mode = IM_LMIC
    {
		if((bm==BM_LR)||(bm==BM_L))
			leftLineLeft(!bypass);
		if((bm==BM_LR)||(bm==BM_R))
			rightMic1(!bypass);
    }

    //connect/disconnect the line/mic input to output mixer
    if(getInputMode() == 0) //mode = IM_LR
    {      
		if((bm==BM_LR)||(bm==BM_L))
			omixerLeftLineLeft(bypass);
		if((bm==BM_LR)||(bm==BM_R))
			omixerRightLineRight(bypass);
    }
    else //inputMode = IM_LMIC
    {
		if((bm==BM_LR)||(bm==BM_L))
			omixerLeftLineLeft(bypass);
		if((bm==BM_LR)||(bm==BM_R))
			omixerRightMic1(bypass);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////
//ES8388 codec class
bool ES8388Codec::writeReg(uint8_t reg, uint16_t val)
{
	if(_codecWire == NULL)
		return false;
	_codecWire->beginTransmission(i2cAddress);
	_codecWire->write(reg);
	_codecWire->write(uint8_t(val & 0xff));
	return (0 == _codecWire->endTransmission(true));
}
uint16_t ES8388Codec::readReg(uint8_t reg)
{
	_codecWire->beginTransmission(i2cAddress);
	_codecWire->write(reg);
	_codecWire->endTransmission(false);
	uint16_t val = 0u;
	if (1 == _codecWire->requestFrom(uint16_t(i2cAddress), uint8_t(1), true))
	{
		val = uint16_t(_codecWire->read());
	}
	_codecWire->endTransmission(false);
	return val;
}; 

bool ES8388Codec::init(int address)
{
	outCorrectionGain = 1.05924; //correction for -0.5 missmatch of always-on ALC gain
	i2cAddress = address;
	bool res = true;

    //INITIALIZATION (BASED ON ES8388 USER GUIDE EXAMPLE)
    //BEGIN STEP BY STEP
    // Set Chip to Slave
	res &= writeReg(ES8388_MASTERMODE,0x00); 
	delay(10);
	
	//Power down DEM and STM
	res &= writeReg(ES8388_CHIPPOWER,0xF3);
	delay(10);
	
	//Set same LRCK	Set same LRCK
	res &= writeReg(ES8388_DACCONTROL21,0x80);
	delay(10);
	
	//Set Chip to Play&Record Mode
	res &= writeReg(ES8388_CONTROL1,0x05);
	delay(10);
	
	//Power Up Analog and Ibias
	res &= writeReg(ES8388_CONTROL2,0x40);
	delay(10);
	//END STEP BY STEP
	
	//Power up ADC / Analog Input /
	//Micbias for Record
	res &= writeReg(ES8388_ADCPOWER,0x00);

	//Power up DAC and Enable LOUT/ROUT
	//res &= writeReg(ES8388_DACPOWER, 0x3C,true); //LR 1,2 out enable
	//res &= writeReg(ES8388_DACPOWER, 0x0C,true);  //LR 2 out enable
	res &= writeReg(ES8388_DACPOWER, 0x30);  //LR 1 out enable
	
	//ADC SETUP
	
	//Select Analog input channel for ADC
	if(getInputMode()==0) //LR mode
	{
		res &= writeReg(ES8388_ADCCONTROL2,0x50);// LINSEL:LINPUT2, RINSEL:RINPUT2,
		//res &= writeReg(ES8388_ADCCONTROL2,0x00);// LINSEL:LINPUT1, RINSEL:RINPUT1,
		//res &= writeReg(ES8388_ADCCONTROL2,0x10);// LINSEL:LINPUT1, RINSEL:RINPUT2,
		//res &= writeReg(ES8388_ADCCONTROL2,0x40);// LINSEL:LINPUT2, RINSEL:RINPUT1,
		res &= writeReg(ES8388_ADCCONTROL3,0x02);//DS(DIFF SELECT): LINPUT1-RINPUT1, STEREO, ASDOUT NORMAL
		
		//ALC SETUP
		res &= writeReg(ES8388_ADCCONTROL10,0xC8); //1100 1000, ALC:LR, MAXGAIN:-0.5dB, MINGAIN:-12dB
		res &= writeReg(ES8388_ADCCONTROL11,0xA0); //1010 0000, ALC Target:-1.5dB, hold time:0
		res &= writeReg(ES8388_ADCCONTROL12,0x12); //0001 0010, limiter ramp up: 182 uS, limiter ramp down: 90.8 uS
		res &= writeReg(ES8388_ADCCONTROL13,0x06); //0000 0110, alc mode: ALC, z.cross disable, disable z.c. timeout, ramp up: 182 uS, peak detect window: 96 samples
		res &= writeReg(ES8388_ADCCONTROL14,0x00); //0000 0000, disable noise gate
	}
	else //LMIC mode
	{
		res &= writeReg(ES8388_ADCCONTROL2,0x74);// LINSEL:LINPUT2, RINSEL: LR-DIFF, 
													//DSSEL:USE ONE DS(DSL), DSR: LINPUT2-RINPUT2
		res &= writeReg(ES8388_ADCCONTROL3,0x02);//DSL: LINPUT1-RINPUT1, STEREO, ASDOUT NORMAL
		
		//ALC SETUP
		res &= writeReg(ES8388_ADCCONTROL10,0x58); //0101 1000, ALC:R, MAXGAIN:11.5dB, MINGAIN:-12dB
		res &= writeReg(ES8388_ADCCONTROL11,0xA0); //1010 0000, ALC Target:-1.5dB, hold time:0
		res &= writeReg(ES8388_ADCCONTROL12,0x12); //0001 0010, limiter ramp up: 182 uS, limiter ramp down: 90.8 uS
		res &= writeReg(ES8388_ADCCONTROL13,0x06); //0000 0110, alc mode: ALC, z.cross disable, disable z.c. timeout, ramp up: 182 uS, peak detect window: 96 samples
		res &= writeReg(ES8388_ADCCONTROL14,0x01); // Reg 0x16 = 0x01(noise gate = -76.5dB, NGG = 0x00(PGA gain held constant))
	
	}
	
	//Select Analog Input PGA Gain for ADC
	res &= writeReg(ES8388_ADCCONTROL1,0x00); //(0dB for both left and right PGA)
	
	//Set SFI for ADC
	//res &= writeReg(ES8388_ADCCONTROL4,0x00); // (I2S – 24Bit)
	res &= writeReg(ES8388_ADCCONTROL4,0x20); // (I2S – 24Bit, LR INVERTED)
	
	//Select MCLK / LRCK ratio for ADC
	//res &= writeReg(ES8388_ADCCONTROL5,0x02);// (256, single speed)
	//res &= writeReg(ES8388_ADCCONTROL5,0x03);// (384, single speed)
	res &= writeReg(ES8388_ADCCONTROL5,0x23);// (384, double speed)
	
	//Set ADC Digital Volume
	res &= writeReg(ES8388_ADCCONTROL8,0x00);// (0dB)
	res &= writeReg(ES8388_ADCCONTROL9,0x00);//(0dB)
  
    //DAC SETUP
    
	//Select ALC for ADC Record (Reg 0x12 to Reg 0x16)
	//Refer to ALC description (default: ALC and noise gate disabled)
	//{}

	//Set SFI for DAC
	//res &= writeReg(ES8388_DACCONTROL1,0x00);// (I2S – 24Bit)
	res &= writeReg(ES8388_DACCONTROL1,0x40);// (I2S – 24Bit, LR INVERTED)
	
	//Select MCLK / LRCK ratio for	DAC
	//res &= writeReg(ES8388_DACCONTROL2,0x02);// (256 single speed)
	//res &= writeReg(ES8388_DACCONTROL2,0x03);// (384, single speed)
	res &= writeReg(ES8388_DACCONTROL2,0x23);// (384, double speed)
	
	//Set DAC Digital Volume
	res &= writeReg(ES8388_DACCONTROL4,0x00);// (0dB)
	res &= writeReg(ES8388_DACCONTROL5,0x00);// (0dB)
	
	//Enable 44.1kHz Deemphasis on single speed, enable click free on power up n down
	//res &= writeReg(ES8388_DACCONTROL6,0x88);// (0dB)
	
	//Setup Mixer, Please refer to Mixer description
	res &= writeReg(ES8388_DACCONTROL16,0x1B); //left in select for out mix: L-ADC-IN, Right in select for outmix: R-ADC-IN
	res &= writeReg(ES8388_DACCONTROL17,0x90); //left mixer input from left dac only, gain = 0dB
	res &= writeReg(ES8388_DACCONTROL20,0x90); //right mixer input from right dac only, gain = 0dB
	
	//Set LOUT/ROUT Volume
	res &= writeReg(ES8388_DACCONTROL24,0x1E);// L1 (0dB)
	res &= writeReg(ES8388_DACCONTROL25,0x1E);// R1 (0dB)
	res &= writeReg(ES8388_DACCONTROL26,0x1E);// L2 (0dB)
	res &= writeReg(ES8388_DACCONTROL27,0x1E);// R2 (0dB)
	
	//optimize A/D conversion for 1/4 Vrms range
	optimizeConversion(2);
	
	//Power up DEM and STM
	res &= writeReg(ES8388_CHIPPOWER,0x00);
	return res;
}
	
//get and set the output level (analog gain)
//vol = 0-31
bool ES8388Codec::setOutVol(int vol)
{
	if(vol>30) vol = 30;
	bool res = true;
	res &= writeReg(ES8388_DACCONTROL24, vol); //LOUT1VOL
    res &= writeReg(ES8388_DACCONTROL25, vol); //ROUT1VOL
    //res &= writeReg(ES8388_DACCONTROL26, vol); //LOUT2VOL
    //res &= writeReg(ES8388_DACCONTROL27, vol); //ROUT2VOL
	return res;
}

int ES8388Codec::getOutVol()
{
	return readReg(ES8388_DACCONTROL24);
}

bool ES8388Codec::setInGain(int gain)
{
	if(gain > 8) gain = 8;
	uint8_t temp = readReg(ES8388_ADCCONTROL1);
		
	if(getInputMode()==0) //input mode = IM_LR
	{
		temp = gain << 4;
		temp = temp | gain;
	}
	else //input mode = IM_LMIC
	{
		temp = 0x0F & temp;
		temp = temp | (gain << 4);
	}
	return writeReg(ES8388_ADCCONTROL1,temp);
}

int ES8388Codec::getInGain()
{
	uint8_t temp = readReg(ES8388_ADCCONTROL1);
	temp = (temp & 0xF0) >> 4;
	return temp;
}

void ES8388Codec::optimizeConversion(int range)
{
	int ingain[]={0, 2, 4, 6, 8}; //0db, 6dB, 12dB, 18dB, 24dB
	int outvol[]= {30, 26, 22, 18, 14}; //0db, -6dB, -12dB, -18dB, -24dB
	if(range<0) range = 0;
	if(range>4) range = 4;
	setOutVol(outvol[range]);
	setInGain(ingain[range]);
}
	
//get and set microphone gain (0:0dB,1-7:30dB-48dB)
uint8_t ES8388Codec::getMicGain()
{
	if(getInputMode()==1) //input mode = LMIC
		return (0x0F & readReg(ES8388_ADCCONTROL1));
	else return 0;
}

bool ES8388Codec::setMicGain(uint8_t gain)
{	
	if(getInputMode()==1) //input mode = LMIC
	{
		if(gain > 8) gain = 8;
		uint8_t temp = readReg(ES8388_ADCCONTROL1);
		temp = temp & 0xF0;
		temp = temp | gain;
		return writeReg(ES8388_ADCCONTROL1,temp); 
	}
	else return false;
}

int ES8388Codec::getMicNoiseGate()
{
	if(getInputMode()==1)
	{
		uint8_t temp = readReg(ES8388_ADCCONTROL14);
		temp = temp >> 3;
		return (int) temp;
	}
	else return 0;
}

bool ES8388Codec::setMicNoiseGate(int gate)
{
	if(getInputMode()==1) //input mode = IM_LMIC
	{
		if(gate > 32) gate = 32;
		if(gate>0)
		{
			uint8_t temp = ((gate-1) << 3) | 1;
			return writeReg(ES8388_ADCCONTROL14,temp); 
		}
		else //turn off the noise gate at gate = 0
		{
			return writeReg(ES8388_ADCCONTROL14,0); 
		}
	}
	else return false;
}

//bypassed the analog input to the output, disconnect the digital i/o 
bool ES8388Codec::analogBypass(bool bypass, BYPASS_MODE bm)
{
	bool res = true;
	if(bypass)
	{
		if((bm==BM_LR)||(bm==BM_L))
			*muteLeftAdcIn = true;
		if((bm==BM_LR)||(bm==BM_R))
			*muteRightAdcIn= true;
		
		if((bm==BM_LR)||(bm==BM_L))
			res &= writeReg(ES8388_DACCONTROL17,0x50); //disable ldac, enable lin, gain = 0dB
		if((bm==BM_LR)||(bm==BM_R))
			res &= writeReg(ES8388_DACCONTROL20,0x50); //disable rdac, enable rin, gain = 0dB
	}
	else
	{
		if((bm==BM_LR)||(bm==BM_L))
			*muteLeftAdcIn = false;
		if((bm==BM_LR)||(bm==BM_R))
			*muteRightAdcIn= false;
		
		if((bm==BM_LR)||(bm==BM_L))
			res &= writeReg(ES8388_DACCONTROL17,0x90); //enable ldac,disable lin, gain = 0dB
		if((bm==BM_LR)||(bm==BM_R))
			res &= writeReg(ES8388_DACCONTROL20,0x90); //enable rdac,disable rin, gain = 0dB
	}	
	return res;
}

//bypassed the analog input to the output, disconnect the digital input, preserve the digital output connection
bool ES8388Codec::analogSoftBypass(bool bypass, BYPASS_MODE bm)
{
	bool res = true;
	if(bypass)
	{
		if((bm==BM_LR)||(bm==BM_L))
			*muteLeftAdcIn = true;
		if((bm==BM_LR)||(bm==BM_R))
			*muteRightAdcIn= true;
		
		if((bm==BM_LR)||(bm==BM_L))
			res &= writeReg(ES8388_DACCONTROL17,0xD0); //enable ldac, enable lin, gain = 0dB
		if((bm==BM_LR)||(bm==BM_R))
			res &= writeReg(ES8388_DACCONTROL20,0xD0); //enable rdac, enable rin, gain = 0dB
	}
	else
	{
		if((bm==BM_LR)||(bm==BM_L))
			*muteLeftAdcIn = false;
		if((bm==BM_LR)||(bm==BM_R))
			*muteRightAdcIn= false;
		
		if((bm==BM_LR)||(bm==BM_L))
			res &= writeReg(ES8388_DACCONTROL17,0x90); //enable ldac,disable lin, gain = 0dB
		if((bm==BM_LR)||(bm==BM_R))
			res &= writeReg(ES8388_DACCONTROL20,0x90); //enable rdac,disable rin, gain = 0dB
	}
	return res;
}

