/*!
 *  @file       control.cpp
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
  
#include "control.h"

void controltask(void *arg)
{
  controlInterface *con = (controlInterface*) arg;
  int tempocounter = 0;
  while(true)
  {
    vTaskDelay(1);
    con->runningTicks++;
    
    if(con->runningTicks < 200) //stabilize the filter and skip the routine
    {
      for(int i=0;i<6;i++)
      {
        float val = analogRead(con->controlPin[i]);
        if(con->module->control[i].inverted)
          val = 4095-val;
        if(con->module->control[i].slowSpeed)
			con->slowLpf[i]->process((const float*)&val, &val, 1);
		else 
			con->lpf[i]->process((const float*)&val, &val, 1);
      }
      continue; //skip the routine
    }
    
    for(int i=0;i<6;i++)
    {
      //read the analog port
      float val = analogRead(con->controlPin[i]);
      if(con->module->control[i].inverted)
        val = 4095-val;

      ////////////////////////////////////////////////////////////////////////////////
      //POTENTIOMETER CONTROL MODE
      if(con->module->control[i].mode == CM_POT)
      {
		//filter the reading
		if(con->module->control[i].slowSpeed)
			con->slowLpf[i]->process((const float*)&val, &val, 1);
		else 
			con->lpf[i]->process((const float*)&val, &val, 1);
		
          int increment = 4096/con->module->control[i].levelCount;
          int position = val/increment;
          
          //normalize the unexpected
          if(position >= con->module->control[i].levelCount)
            position = con->module->control[i].levelCount -1;
          if(position < 0) position = 0;

          //add some hysteresis for stability
          int modulus = (int)val % increment;
          if(position > con->module->control[i].value)
          {
            if((position-con->module->control[i].value > 1)||(modulus > (increment>>2)))
            {
              con->module->control[i].value = position;
              con->module->onControlChange(i);
            }
          }
          else if(position < con->module->control[i].value)
          {
            if(((con->module->control[i].value - position) > 1)||(modulus < (increment-(increment>>2))))
            {
              con->module->control[i].value = position;
              con->module->onControlChange(i);
            }
          }
      }
      ////////////////////////////////////////////////////////////////////////////////
      //SELECTOR CONTROL MODE
      else if(con->module->control[i].mode == CM_SELECTOR)
      {
        //filter the reading
        con->lpf[i]->process((const float*)&val, &val, 1);
        
        //find the selector channel from val
        int channelwidth = 4096/con->module->control[i].levelCount;
        int readchannel = val/channelwidth;

        //normalize the unexpected
        if(readchannel >= con->module->control[i].levelCount)
          readchannel = con->module->control[i].levelCount -1;
        if(readchannel < 0) readchannel = 0;
          
        if(readchannel != con->module->control[i].value) //the value has changed
        {
          con->module->control[i].value = readchannel;
          con->module->onControlChange(i);
        }
            
      }
      ////////////////////////////////////////////////////////////////////////////////
      //TOGGLE PUSH BUTTON CONTROL MODE
      else if(con->module->control[i].mode == CM_TOGGLE)
      {
        switch(con->controlState[i])
        {
          case 0: //wait press
          {
            if(val < 2048)
            {
              con->controlState[i] = 1; //wait stable press
              con->stateCounter[i]=0;
            }
            break;
          }
          case 1: //wait stable press
          {
            if(val < 2048)
            {
              con->stateCounter[i]++;
              if(con->stateCounter[i] > 2)
              {
                if(con->module->control[i].value==0)
                  con->module->control[i].value=1;
                else con->module->control[i].value=0;
                
                con->module->onControlChange(i);
                con->unsavedchanges = true;
                con->controlState[i] = 2; //wait release
              }
            }
            else //not < 2048
            {
              con->controlState[i] = 0; //back to wait press
            }
            break;
          }
          case 2: //wait release
          {
            if(val > 2048)
            {
              con->controlState[i] = 3; //wait stable release
              con->stateCounter[i] = 0;
            }
          }
          case 3: //wait stable release
          {
            if(val > 2048)
            {
              con->stateCounter[i]++;
              if(con->stateCounter[i] > 2)
              {
                con->controlState[i] = 0; //back to wait press
              }
            }
            else con->controlState[i] = 2; //back to wait release
            break;
          }
        }
      }
      ////////////////////////////////////////////////////////////////////////////////
      //MOMENTARY PUSH BUTTON CONTROL MODE
      else if(con->module->control[i].mode == CM_MOMENTARY)
      {
          int tempval = 0;
          if(val < 2048)
            tempval = 1;
          if(con->module->control[i].value == tempval)
            con->stateCounter[i]=0;
          else
          {
            con->stateCounter[i]++;
            if(con->stateCounter[i] > 2)
            {
              con->module->control[i].value = tempval;
              con->module->onControlChange(i);
            }
          }
      }
      ////////////////////////////////////////////////////////////////////////////////
      //TAP TEMPO CONTROL MODE
      else if(con->module->control[i].mode == CM_TAPTEMPO)
      {
        switch(con->controlState[i])
        {
          case 0: //wait first tap
          {
            if(val < 2048)
            {
              con->controlState[i] = 1; //wait release, no need to wait stable press
              tempocounter = 0; //start tempo couter immediately
              con->module->auxLed->turnOn();
            }
            break;
          }
          case 1: //wait release
          {
            tempocounter++;
            if(tempocounter > con->module->control[i].max) //if expired
            {
              con->module->auxLed->blink(10,con->module->control[i].value-10,1,0,0);
              con->controlState[i]=0; //back to start
            }
            else if(val > 2048)
            {
              con->controlState[i] = 2; //wait stable release
              con->stateCounter[i]=0;
            }
            break;
          }
          case 2: //wait stable release
          {
            tempocounter++;
            if(tempocounter > con->module->control[i].max) //if expired
            {
              con->module->auxLed->blink(10,con->module->control[i].value-10,1,0,0);
              con->controlState[i]=0; //back to start
            }
            else if(val > 2048) //if released
            {
              con->stateCounter[i]++;
              if(con->stateCounter[i] > 10)
              {
                con->controlState[i]=3; //wait second tap
              }
            }
            else //not release
              con->stateCounter[i] = 0; //reset the stable count
            break;
          }
          case 3: //wait second tap
          {
            tempocounter++;
            if(tempocounter > con->module->control[i].max) //if expired
            {
              con->module->auxLed->blink(10,con->module->control[i].value-10,1,0,0);
              con->controlState[i]=0; //back to start
            }
            else if(val < 2048) //tapped
            {
				if(tempocounter < con->module->control[i].min)
					tempocounter = con->module->control[i].min;
				con->module->auxLed->blink(10,tempocounter-10,1,0,0);
				con->module->control[i].value = tempocounter;
				con->module->onControlChange(i);
				con->stateCounter[i] = 0;
				con->controlState[i] = 4; //wait the second stable release
            }
            break;
          }
          case 4: //wait the second  stable release
          {
            if(val > 2048)
            {
              con->stateCounter[i]++;
              if(con->stateCounter[i] > 10)
              {
                con->controlState[i] = 0; //back to wait the first tap
              }
            }
            else con->stateCounter[i]=0;
            
            break;
          }
        }//end switch
      }
      else //mode = CM_DISABLED
      {
        //do nothing
      }
    }
  }
}

controlInterface::controlInterface()
{
  runningTicks = 0;
  unsavedchanges = false;
  for(int i=0;i<6;i++)
  {
    controlState[i]=0;

    //fourth order filter setup 10 Hz cut off at 1k samples/s
    lpf[i] = new biquadFilter(2);
    //set the coefficients for 10 Hz cut off frequency at 1k sample/s
    const float coefficients[] = 
    {
      //these coefficients are calculated using online digital filter design tool https://www.micromodeler.com/dsp/
      //10 Hz
      0.0009200498139105926, 0.0018400996278211852, 0.0009200498139105926, 1.8866095826215064, -0.8903397362840242, // b0, b1, b2, a1, a2
      0.0009765625, 0.001953125, 0.0009765625, 1.9492159580258417, -0.9530698953278909 //  b0, b1, b2, a1, a2
    };
    lpf[i]->setCoef(coefficients);
    
    //fourth order filter setup 4 Hz cut off at 1k samples/s
    slowLpf[i] = new biquadFilter(2);
    const float coefficients2[] = 
    {
      //these coefficients are calculated using online digital filter design tool https://www.micromodeler.com/dsp/    
      //4 Hz
		0.00019772393992324234, 0.0003954478798464847, 0.00019772393992324234, 1.954001961679803, -0.9546192513864591,// b0, b1, b2, a1, a2
		0.0001220703125, 0.000244140625, 0.0001220703125, 1.980323859118934, -0.9809494641889661// b0, b1, b2, a1, a2
    };
    slowLpf[i]->setCoef(coefficients2);
  }  
}

controlInterface::~controlInterface()
{
  for(int i=0;i<6;i++)
  {
    delete lpf[i];
    delete slowLpf[i];
  }
}

void controlInterface::init(int p1pin, int p2pin, int p3pin, int p4pin, int p5pin, int p6pin, int priority)
{
  controlPin[0]=p1pin;
  controlPin[1]=p2pin;
  controlPin[2]=p3pin;
  controlPin[3]=p4pin;
  controlPin[4]=p5pin;
  controlPin[5]=p6pin;
  xTaskCreatePinnedToCore(controltask, "controltask",4096,(void*)this,priority,NULL,0);
}
