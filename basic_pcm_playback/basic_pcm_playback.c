/*******************************************************************************
 * @file          basic_pcm_playback.c
 *
 * @brief         Simple client for the ALSA playback
 *
 * Simple client for the ALSA playback
 *
 * @note          Link using -lasound and -lm, -lalsa_utils,
 *                -L${workspace_loc:/alsa_utils/Debug/} and
 *                -I${workspace_loc:/alsa_utils}, based on the HOWTO
 *                 https://users.suse.com/~mana/alsa090_howto.html
 *
 * @author        hkxs
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

/*------------------------------------------------------------------------------
 * Includes
 -----------------------------------------------------------------------------*/
#include <alsa/asoundlib.h>
#include "alsa_utils.h"

/*------------------------------------------------------------------------------
 * Module Preprocessor Constants
 -----------------------------------------------------------------------------*/
#define FREQUENCY               (469u) /**< I choose this frequency to test
                                            because for 2048samples at 48Khz the
                                             mismatch between frames will be
                                              minimized */

/*------------------------------------------------------------------------------
 * Module Typedefs
 -----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Function Prototypes
 -----------------------------------------------------------------------------*/

/******************************************************************************
 *
 * @fn int main (void)
 *
 * @brief Main function of the program
 *
 * Main function of the program
 *
 * @return int  @a S_SUCCESS in case of success, @a S_ERROR otherwise
 *
 ******************************************************************************/
int main (void)
{
  /** @b snd_pcm_t  pcm_handle  Handle for the PCM device */
  snd_pcm_t *pcm_handle;

  /* Direction of the stream, to play or record */
  snd_pcm_stream_t stream_direction = SND_PCM_STREAM_PLAYBACK;

  /** @b hw_configuration Hardware configuration that we want
   * @li sample rate = 48KHz
   * @li frame size = 4800bytes
   * @li access type = non interleaved, this means that each period contains
   *                   first all sample data for the first channel followed by
   *                   the sample data for the second channel */
  hw_configuration hw_configuration = { .sample_rate = 48000u, .periods = 2,
      .period_size = 2048, .sample_rate_direction = E_EXACT_CONFIG,
      .access_type = SND_PCM_ACCESS_RW_INTERLEAVED, .num_channels = 2,
      .frame_size_direction = E_EXACT_CONFIG, .format = SND_PCM_FORMAT_S16_LE };

  /** @b pcm_name Name of the PCM device, like @a plughw:0,0
   * @li The first number is the number of the soundcard
   * @li The second number is the number of the device */
  char *pcm_name = "default"; /* Use default system audio card */

  int8_t err = 0u;

  /** @b snd_pcm_open Create a handle and open a connection to a specified
   * audio interface, this function receives as arguments:
   * 1. pcmp: handle for the audio interface
   * 2. name: name of the sound card like @a plughw:0,0
   * 3. stream: pcm stream direction (it can be SND_PCM_STREAM_PLAYBACK or
   *            SND_PCM_STREAM_CAPTURE)
   * 4. mode: open mode for the sound card, set to 0 use standard (blocked) mode
   *          in this mode, if the resource is busy it will block the caller
   *          until the resource is freed, setting it to @a SND_PCM_NONBLOCK
   *          doesn't block the caller in any way and returns -EBUSY error when
   *          the resources are not available, if set to @a SND_PCM_ASYNC to
   *          receive asynchronous notification after specified time periods
   * After successfully calling this function the sound card should be in
   * @a SND_PCM_STATE_OPEN state */
  err = snd_pcm_open (&pcm_handle, pcm_name, stream_direction,
  PCM_OPEN_STANDARD_MODE);
  if ( S_SUCCESS>err )
  {
    printf ("Error opening sound card, Err = %d\n", err);
    return S_ERROR;
  }

  err = configure_hw (pcm_handle, &hw_configuration);
  if ( S_SUCCESS>err )
  {
    printf ("Unable to configure HW, Err = %d\n", err);
    snd_pcm_close (pcm_handle);

    return S_ERROR;
  }

  /* Now it's time to generate a signal to test the output of the sound card */
  int16_t *sine_wave;
  uint32_t sine_size;
  uint8_t number_of_frames = 46; /*40*2048/4800 = 1.96s ~= 2s*/

  printf ("Generating random noise\n");
  sine_size = hw_configuration.period_size*hw_configuration.periods;
  sine_wave = (int16_t*)malloc (sizeof(sine_wave)*sine_size);

  if ( NULL==sine_wave )
  {
    printf ("Error allocating memory for the audio signal\n");
    snd_pcm_close (pcm_handle);

    return S_ERROR;
  }

  err = generate_sin (sine_wave, FREQUENCY, hw_configuration.sample_rate,
                      sine_size);
  if ( S_ERROR==err )
  {
    printf ("Error generating sine wave\n");
    free (sine_wave);
    snd_pcm_close (pcm_handle);

    return S_ERROR;
  }
  printf ("Sending data to sound card\n");

  /** @b snd_pcm_writei With everything set we can start writing data the API
   *  is different depending of the access_type:
   * @li snd_pcm_writei for SND_PCM_ACCESS_MMAP_INTERLEAVED
   * @li snd_pcm_writen for SND_PCM_ACCESS_MMAP_NONINTERLEAVED */
  for (uint8_t i = 0u; i<number_of_frames; i++)
  {
    err = snd_pcm_writei (pcm_handle, sine_wave, hw_configuration.period_size);

    /* if we fail we try to recover the stream state*/
    if ( S_SUCCESS>err )
    {
      err = snd_pcm_recover (pcm_handle, err, 1);
    }

    /* if we fail from recovery we suspend everything*/
    if ( S_SUCCESS>err )
    {
      printf ("Error writing data to the sound card\n");
      free (sine_wave);
      snd_pcm_close (pcm_handle);

      return S_ERROR;
    }
  }

  free (sine_wave);
  /* close the sound card */
  snd_pcm_close (pcm_handle);

  return S_SUCCESS;
}

/*-------------- END OF FILE -------------------------------------------------*/
