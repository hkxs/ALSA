/*******************************************************************************
 * @file         : basic_pcm.c
 * @brief        : Simple client for the ALSA sequencer
 *
 * @description  : Simple client for the ALSA sequencer
 *
 * @notes        : Link using -lasound, based on the HOWTO
 *                 https://users.suse.com/~mana/alsa090_howto.html
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
#define S_SUCCESS               (0x00)
#define S_ERROR                 (0x01)
#define PCM_OPEN_STANDARD_MODE  (0x00)

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

int main (void)
{
  /* Handle for the PCM device */
  snd_pcm_t *pcm_handle;

  /* Direction of the stream, to play or record */
  snd_pcm_stream_t stream_direction = SND_PCM_STREAM_PLAYBACK;

  /* This structure contains information about    */
  /* the hardware and can be used to specify the  */
  /* configuration for the PCM stream. */
  snd_pcm_hw_params_t *hw_params;

  /* Name of the PCM device, like \a plughw:0,0          */
  /* The first number is the number of the soundcard, */
  /* the second number is the number of the device.   */
  char *pcm_name = "hw:1,0"; /* Rear headphones, obtained from audacity */

  uint8_t err = 0u;

  /* Allocate snd_pcm_hw_params_t structure on the stack.
   * @note we can also use \a snd_pcm_hw_params_malloc() to allocate the
   * memory, but we'll be responsible for free it when we finish using
   * \a snd_pcm_hw_params_free() */
  snd_pcm_hw_params_alloca(&hw_params);

  /* Create a handle and open a connection to a specified audio interface
   * This function receives as arguments:
   * 1. pcmp: handle for the audio interface
   * 2. name: name of the sound card like \a plughw:0,0
   * 3. stream: pcm stream direction (it can be SND_PCM_STREAM_PLAYBACK or
   *            SND_PCM_STREAM_CAPTURE)
   * 4. mode: open mode for the sound card, set to 0 use standard (blocked) mode
   *          in this mode, if the resource is busy it will block the caller
   *          until the resource is freed, setting it to \a SND_PCM_NONBLOCK
   *          doesn't block the caller in any way and returns -EBUSY error when
   *          the resources are not available, if set to \a SND_PCM_ASYNC to
   *          receive asynchronous notification after specified time periods
   * After succesfully calling this funciton the sound card should be in
   * \a SND_PCM_STATE_OPEN state */
  err = snd_pcm_open (&pcm_handle, pcm_name, stream_direction,
  PCM_OPEN_STANDARD_MODE);
  if (S_SUCCESS != err)
  {
    printf ("Error opening sound card\n");
    return S_ERROR;
  }

}

/*************** END OF FILE **************************************************/
