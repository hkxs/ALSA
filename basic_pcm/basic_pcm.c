/*******************************************************************************
* @file         : basic_pcm.c
* @brief        : Simple client for the ALSA sequencer
*
* @description  : Simple client for the ALSA sequencer
*
* @notes        : Based on the HOWTO https://users.suse.com/~mana/alsa090_howto.html
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
#include <alsa/asoundlib.h>

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/


/******************************************************************************
* Function Prototypes
*******************************************************************************/


void main(void)
{
  /* Handle for the PCM device */
  snd_pcm_t *pcm_handle;

  /* Playback stream */
  snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;

  /* This structure contains information about    */
  /* the hardware and can be used to specify the  */
  /* configuration to be used for the PCM stream. */
  snd_pcm_hw_params_t *hwparams;

  /* Name of the PCM device, like plughw:0,0          */
  /* The first number is the number of the soundcard, */
  /* the second number is the number of the device.   */
  char *pcm_name = "plughw:3,0"; /* USB headphones, obtained fro audacity */

}


/*************** END OF FILE **************************************************/
