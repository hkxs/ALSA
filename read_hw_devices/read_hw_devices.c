/*******************************************************************************
* @file         : read_hw_devices.c
* @brief        : this program will list the audio devices found
*
* @description  :
*
* @notes        : link using -lasound
*
* @author       : hkxs
*
*
* MIT License
*
* Copyright (c) 2020 Hkxs
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*******************************************************************************/


/******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>
#include <alsa/asoundlib.h>

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/


/******************************************************************************
* Function Prototypes
*******************************************************************************/


/******************************************************************************
*
* @brief Main function
*
* @description
*
* @param None
*
* @return return 0 in success
*
*******************************************************************************/
int main(void)
{

  printf("ALSA library version: %s\n", SND_LIB_VERSION_STR);

}

/*************** END OF FILE **************************************************/
