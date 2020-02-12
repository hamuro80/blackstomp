/*
  * Program Sketch for Blackstomp ESP32-A1S Digital Effect Board
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
#include "ac101.h"
#include "driver/i2s.h"
#include "esp_task_wdt.h"

#define SAMPLE_RATE     (44100)
#define I2S_NUM         (0)
#define I2S_BCK_IO      (GPIO_NUM_27)
#define I2S_WS_IO       (GPIO_NUM_26)
#define I2S_DO_IO       (GPIO_NUM_25)
#define I2S_DI_IO       (GPIO_NUM_35)
#define CODEC_SCK       (GPIO_NUM_32)
#define CODEC_SDA       (GPIO_NUM_33)

//audio processing frame length in samples (L+R) 64 samples (32R+32L) 256 Bytes
#define FRAMELENGTH    64
//sample count per channel for each frame (32)
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
#define DMABUFFERCOUNT  20

//codec instance
AC101 codec;

//buffers
float wleft[] = {0,0};
float wright[] = {0,0};
int32_t inbuffer[FRAMELENGTH];
int32_t outbuffer[FRAMELENGTH];
float inleft[FRAMESIZE];
float inright[FRAMESIZE];
float outleft[FRAMESIZE];
float outright[FRAMESIZE];
unsigned long processedframes;

//DIGITAL SIGNAL PROCESSING FUNCTION
void dsprocess(float* left_in, float* right_in, float* left_out, float* right_out, int samplecount)
{
  //BEGIN WRITE YOUR DIGITAL PROCESSING ALGORITHM HERE
  for(unsigned int i=0;i<samplecount;i++)
  {
    left_out[i] = left_in[i];
    right_out[i] = right_in[i];
  }
  //END WRITE YOUR DIGITAL PROCESSING ALGORITHM HERE
}

void i2s_task(void* arg)
{
  size_t bytesread, byteswritten;
  processedframes = 0;

  //initialize all output buffer to zero
  for(int i= 0; i< FRAMELENGTH; i++)
    outbuffer[i] = 0;
  for(int i=0; i< SAMPLECOUNT; i++)
  {
    outleft[i]=0;
    outright[i]=0;
  }
  
  while(true)
  {
    processedframes++;
    i2s_read((i2s_port_t)I2S_NUM,(void*) inbuffer, FRAMESIZE, &bytesread, 20);
    //convert from int to float
    for(int i=0,k=0;k<SAMPLECOUNT;k++,i+=2)
    {
      inleft[k] = (float) inbuffer[i];  
      //inright[k] = (float) inbuffer[i+1];
      inright[k] = inleft[k];
    }
  
    //process the signal
    dsprocess(inleft, inright, outleft, outright, SAMPLECOUNT);
    
    //convert back float to int
    for(int i=0,k=0;k<SAMPLECOUNT;k++,i+=2)
    {
      outbuffer[i] = (int32_t) outleft[k];
      outbuffer[i+1] = (int32_t) outright[k];
    }
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
  i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  i2s_config.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB);
  i2s_config.dma_buf_count = DMABUFFERCOUNT;
  i2s_config.dma_buf_len = DMABUFFERLENGTH;
  i2s_config.use_apll = false;
  i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1 ;
  
  i2s_pin_config_t pin_config;
  pin_config.bck_io_num = I2S_BCK_IO;
  pin_config.ws_io_num = I2S_WS_IO;
  pin_config.data_out_num = I2S_DO_IO;
  pin_config.data_in_num = I2S_DI_IO;

  i2s_driver_install((i2s_port_t)I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin((i2s_port_t)I2S_NUM, &pin_config);
  i2s_set_clk((i2s_port_t)I2S_NUM, SAMPLE_RATE, (i2s_bits_per_sample_t) 32, I2S_CHANNEL_STEREO);
}

void setup() 
{
  //codec setup
  codec.setup(CODEC_SDA, CODEC_SCK);
  
  //stereo unbalanced line-in
  codec.LeftLineLeft(true);
  codec.RightLineRight(true);

/*
  //unbalanced line-in for left channel and balnced microphone for right channel
  codec.LeftLineLeft(true);
  codec.RightMic1(true);
 */

 /*
  //balanced line-in for left channel and balnced microphone for right channel
  codec.LeftLineDiff(true);
  codec.RightMic1(true);
*/
  i2s_setup();
  xTaskCreate(i2s_task, "i2s_task", 4096, NULL, AUDIO_PROCESS_PRIORITY, NULL);
  Serial.begin(115200);
}

void loop() 
{
  Serial.printf("\r\nAudio frames per second: %lu", processedframes);
  processedframes = 0;
  vTaskDelay(1000);
}
