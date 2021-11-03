/*!
 *  @file       effectmodule.cpp
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
 
 #include "effectmodule.h"

 effectModule::effectModule()
 {
   name = "My Effect";
   
   inputMode = IM_LR;
   encoderMode = EM_DISABLED;
   
   bleTerminal.servUuid = "d11747ac-6172-4bb1-9b3a-20d58cc88f20";
   bleTerminal.charUuid = "7a9fd763-04a5-4a17-b625-1fce28329f23";
   bleTerminal.passKey = 123456;
   
   for(int i=0;i<6;i++)
   {
    control[i].mode = CM_DISABLED;
    control[i].inverted = false;
    control[i].min = 0;
    control[i].max = 127;
    control[i].levelCount = 128;
    control[i].inverted = false;
    control[i].slowSpeed = false;
   }

   for(int i=0;i<4;i++)
   {
    button[i].inverted = false;
    button[i].value = 0;
    button[i].min = 0;
    button[i].max = 1;
    button[i].mode = BM_DISABLED;
   }
 }

effectModule::~effectModule()
{
  deInit();
}
