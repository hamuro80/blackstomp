#ifndef CODEC_H_
#define CODEC_H_

#include <inttypes.h>

typedef enum
{
	DT_ESP32_A1S_AC101,
	DT_ESP32_A1S_ES8388,
	DT_WROVER_WM8776
} DEVICE_TYPE;

typedef enum
{
	BM_LR,	//bypass both L and R channel (default)
	BM_L,	//bypass L channel only
	BM_R  	//bypass R channel only
} BYPASS_MODE;

//function to init the i2c bus at specific pins and clock frequency
bool codecBusInit(int sdaPin, int sclPin, int frequency);

//function to detect codec chip wired to the i2c pins, return the i2c address if found
//int codecDetect(int sdaPin, int sclPin, int frequency=400000);

//codec, the base class
class codec
{
  protected:
	int i2cAddress;
	virtual bool writeReg(uint8_t reg, uint16_t val){};
	virtual uint16_t readReg(uint8_t reg){};
	
  private:
	int inmode;
	
  public:
  
    float outCorrectionGain;
	bool* muteLeftAdcIn;
	bool* muteRightAdcIn;
	
	//initialize the codec
	virtual bool init(int address){};
	
	//get and set the input mode
	//mode 0: L+R, mode 1: L+MIC
	bool setInputMode(int mode){inmode = mode; return true;};
	int getInputMode(){ return inmode;};
	
	//get and set the output level (analog gain)
	//vol = 0-30 for ES83-version module
	//vol = 0-31 for AC101-version module
	virtual bool setOutVol(int vol){};
	virtual int getOutVol(){};
	
	//get and set the input gain (analog)
	virtual bool setInGain(int gain){};
	virtual int getInGain(){};
	
	//get and set microphone gain (0:0dB,1-7:30dB-48dB)
	virtual uint8_t getMicGain(){};
	virtual bool setMicGain(uint8_t gain){};
	
	//get and set microphone noise gate (0-31: -76.5dB, -75.0dB,...., -30.0dB)
	virtual int getMicNoiseGate(){};
	virtual bool setMicNoiseGate(int gate){};
	
	//optimize the analog to digital conversion range
	//range: 0, 1, 2, 3, 4 (1Vrms/2.83Vpp, 0.5Vrms/1.41Vpp, 0.25Vrms/707mVpp, 0.125Vrms/354mVpp, 0.625Vrms/177mVpp)
	virtual void optimizeConversion(int range=2);
	
	//bypassed the analog input to the output, disconnect the digital i/o 
	virtual bool analogBypass(bool bypass, BYPASS_MODE bm=BM_LR){};  

	//bypassed the analog input to the output, disconnect the digital input, preserve the digital output connection
	virtual bool analogSoftBypass(bool bypass, BYPASS_MODE bm=BM_LR){};  
};

//class for AC101 codec
class AC101Codec:public codec
{
	private:
	bool writeReg(uint8_t reg, uint16_t val);
	uint16_t readReg(uint8_t reg);
	
	bool setI2sWordSize(uint16_t size);
	bool setI2sFormat(uint16_t format);
	bool setI2sMode(uint16_t mode);
	bool setI2sSampleRate(uint16_t rate);
	bool setI2sClock(uint16_t bitClockDiv, uint16_t bitClockInv, uint16_t lrClockDiv, uint16_t lrClockInv);
	
	//leftchannel input selector methods
	bool leftMic1(bool select);		//left channel mic1 select
	bool leftLineDiff(bool select);	//left channel line difference (line Left- Line Right)
	bool leftLineLeft(bool select);	//left channel line (L)

	//left output mixer source select/deselect
	bool omixerLeftLineLeft(bool select);
	bool omixerLeftMic1(bool select);
	bool omixerLeftDacLeft(bool select);

	//right output mixer source select/deselect
	bool omixerRightLineRight(bool select);
	bool omixerRightMic1(bool select);
	bool omixerRightDacRight(bool select);

	//rightchannel input selector methods
	bool rightMic1(bool select);		//right channel mic1 select
	bool rightLineDiff(bool select);	//right channel line difference (line Left- Line Right)
	bool rightLineRight(bool select);	//right channel line (R)
	
	public:
	//initialize the codec
	bool init(int address);
	
	//get and set the output level (analog gain)
	//vol = 0-31
	bool setOutVol(int vol);
	int getOutVol();
	
	//get and set the input gain (analog)
	bool setInGain(int gain){return false;};
	int getInGain(){return 0;};
	
	//optimize analog to digital conversion range
	void optimizeConversion(int range){};
	
	//get and set microphone gain (0:0dB,1-7:30dB-48dB)
	uint8_t getMicGain();
	bool setMicGain(uint8_t gain);
	
	//bypassed the analog input to the output, disconnect the digital i/o 
	bool analogBypass(bool bypass, BYPASS_MODE bm=BM_LR);  

	//bypassed the analog input to the output, disconnect the digital input, preserve the digital output connection
	bool analogSoftBypass(bool bypass, BYPASS_MODE bm=BM_LR);  
};

//class for ES8388 codec
class ES8388Codec:public codec
{
	private:
	bool writeReg(uint8_t reg, uint16_t val);
	uint16_t readReg(uint8_t reg);
	
	public:
	//initialize the codec
	bool init(int address);
	
	//get and set the output level (analog gain)
	//vol = 0-32
	bool setOutVol(int vol);
	int getOutVol();
	
	//get and set the input gain (analog)
	bool setInGain(int gain);
	int getInGain();
	
	//optimize analog to digital conversion range
	void optimizeConversion(int range);
	
	//get and set microphone gain (0:0dB,1-7:30dB-48dB)
	uint8_t getMicGain();
	bool setMicGain(uint8_t gain);
	
	int getMicNoiseGate();
	bool setMicNoiseGate(int gate);
	
	//bypassed the analog input to the output, disconnect the digital i/o 
	bool analogBypass(bool bypass, BYPASS_MODE bm=BM_LR);  

	//bypassed the analog input to the output, disconnect the digital input, preserve the digital output connection
	bool analogSoftBypass(bool bypass, BYPASS_MODE bm=BM_LR);  
};

#endif
