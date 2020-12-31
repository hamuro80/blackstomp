/*!
 *  @file       ledindicator.h
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
#ifndef LEDINDICATOR_H_
#define LEDINDICATOR_H_
#include "Arduino.h"

class ledIndicator
{  
  public:
  void turnOn();
  void turnOff();
  void blink(int onPeriod, int offPeriod,int blinkCount,int repeatCount,int restPeriod);
  void blinkUpdate(int onPeriod, int offPeriod,int blinkCount,int repeatCount,int restPeriod);
  void init(int pin, int priority);
  void deInit();
  unsigned int missedCount; //missing tick by sync wait
  
  private:
  int ledpin;
  int onperiod;
  int offperiod;
  int restperiod;
  int blinks;
  int repeat;
  int onperiodcount;
  int offperiodcount;
  int restperiodcount;
  int blinkcount;
  int repeatcount;
  int runstate;
  int terminaterequest;
  int blinkstate;
  SemaphoreHandle_t xSemaphore;
  friend void blinktask(void* arg);
};

#endif
