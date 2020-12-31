/*!
 *  @file       ledindicator.cpp
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
  
#include "ledindicator.h"

enum{
  blinkstate_turnedoff,blinkstate_turnedon,blinkstate_start,
  blinkstate_on,blinkstate_off,blinkstate_rest
  };

void blinktask(void* arg)
{
  ledIndicator* l = (ledIndicator*) arg;
  
  digitalWrite(l->ledpin,0);
  l->blinkstate = blinkstate_turnedoff;
  l->runstate = 1;
  while(!l->terminaterequest)
  {
    vTaskDelay(10);
    if(xSemaphoreTake(l->xSemaphore,(TickType_t)1) == pdTRUE)
    {
      switch(l->blinkstate)
      {
        case blinkstate_turnedoff:
        {
          digitalWrite(l->ledpin,0);
          break;
        }
        case blinkstate_turnedon:
        {
          digitalWrite(l->ledpin,1);
          break;
        }
        case blinkstate_start:
        {
          digitalWrite(l->ledpin,1);
          l->blinkstate = blinkstate_on;
          l->onperiodcount = 0;
          break;
        }
        case blinkstate_on:
        {
          l->onperiodcount++;
          if(l->onperiodcount >= l->onperiod)
          {
            l->offperiodcount=0;
            digitalWrite(l->ledpin,0);
            l->blinkstate = blinkstate_off;
          }
          break;
        }
        case blinkstate_off:
        {
          l->offperiodcount++;
          if(l->offperiodcount >= l->offperiod) //enough off period
          {
            l->blinkcount ++;
            if(l->blinkcount < l->blinks) //if need more blinks
            {
              l->blinkstate = blinkstate_start;
            }
            else //enough blinks
            {
              l->repeatcount ++;
              if(l->repeat < 1) //repeat forever
              {
                l->blinkstate = blinkstate_rest;
                l->restperiodcount = 0;
              }
              else // not repeat forever
              {
                if(l->repeatcount >= l->repeat) //enough repeat
                  l->blinkstate = blinkstate_turnedoff;
                else //need more repeat
                {
                  l->blinkstate = blinkstate_rest;
                  l->restperiodcount = 0;
                }
              }

            }
          }
          break;
        }
        case blinkstate_rest:
        {
          l->restperiodcount++;
          if(l->restperiodcount > l->restperiod)
          {
            l->blinkcount = 0;
            l->blinkstate = blinkstate_start;
          }
          break;
        }
      }
      //release the semaphore
      
      xSemaphoreGive(l->xSemaphore);
    }
    else //unable to obtain the semaphore
    {
      //record the semaphore sync failure
      l->missedCount++;
    }
  }

  vTaskDelete(NULL);
  pinMode(l->ledpin,INPUT);
  l->runstate = 0;
}

void ledIndicator::init(int pin, int priority)
{
  ledpin = pin;
  pinMode(ledpin,OUTPUT);
  digitalWrite(ledpin,0);
  
  terminaterequest = 0;
  runstate = 1;
  missedCount = 0;
  xSemaphore = xSemaphoreCreateBinary();
  if(xSemaphore!=0)
    xSemaphoreGive(xSemaphore);
    
  xTaskCreatePinnedToCore(blinktask, "blinktask",4096,(void*)this,priority,NULL,0);
}

void ledIndicator::deInit()
{
  terminaterequest = 0;
  while(runstate)
  {
    vTaskDelay(10);
  }
}

void ledIndicator::blink(int onPeriod, int offPeriod,int blinkCount,int repeatCount,int restPeriod)
{
  if(xSemaphoreTake(xSemaphore,(TickType_t)1) == pdTRUE)
  {
    onperiod = onPeriod/10;
    if(onperiod < 1) onperiod=1;

    offperiod = offPeriod/10;
    if(offperiod < 1) offperiod=1;

    restperiod = restPeriod/10;
    if(restperiod < 1) restperiod=1;

    blinks = blinkCount;
    repeat = repeatCount;
    repeatcount = 0;
    blinkcount = 0;
    blinkstate = blinkstate_start;
    
    xSemaphoreGive(xSemaphore);
  }
  else
  {
    missedCount ++;
  }
}

void ledIndicator::blinkUpdate(int onPeriod, int offPeriod,int blinkCount,int repeatCount,int restPeriod)
{
    onperiod = onPeriod/10;
    if(onperiod < 1) onperiod=1;

    offperiod = offPeriod/10;
    if(offperiod < 1) offperiod=1;

    restperiod = restPeriod/10;
    if(restperiod < 1) restperiod=1;

    blinks = blinkCount;
    repeat = repeatCount;
}

void ledIndicator::turnOn()
{
  if(xSemaphoreTake(xSemaphore,(TickType_t)1) == pdTRUE)
  {
    blinkstate = blinkstate_turnedon;
    xSemaphoreGive(xSemaphore);
  }
  else
  {
    missedCount ++;
  }
}

void ledIndicator::turnOff()
{
  if(xSemaphoreTake(xSemaphore,(TickType_t)1) == pdTRUE)
  {
    blinkstate = blinkstate_turnedoff;
    xSemaphoreGive(xSemaphore);
  }
  else
  {
    missedCount ++;
  }
}
