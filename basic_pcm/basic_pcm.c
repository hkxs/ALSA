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
 * Module Typedefs
 *******************************************************************************/
/* @note this enum is used for configuring sample rate and number of periods*/
typedef enum
{
  E_EXACT_CONFIG = 0,/* exact_rate == rate --> dir = 0 */
  E_SMALLER_CONFIG = -1,/* exact_rate < rate  --> dir = -1 */
  E_BIGGER_CONFIG = 1/* exact_rate > rate  --> dir = 1 */
} sub_unit_direction;

typedef struct
{
  uint32_t sample_rate;
  uint32_t exact_sample_rate; /* sample rate set by the sound card */
  sub_unit_direction sample_rate_direction;
  sub_unit_direction frame_size_direction;
  uint32_t frame_size; /* size of the frames in bytes*/
  uint32_t number_of_frames;
  snd_pcm_access_t access_type;
  uint32_t num_channels;
  snd_pcm_format_t format;
} hw_configuration;

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

  /* hardware configuration that we want
   * @li sample rate = 48KHz
   * @li frame size = 1024 bytes
   * @li access type = non interleaved, this means that each period contains
   *                   first all sample data for the first channel followed by
   *                   the sample data for the second channel */
  hw_configuration headphones_hw_configuration = { .sample_rate = 48000u,
      .exact_sample_rate = 0u, .sample_rate_direction = E_EXACT_CONFIG,
      .frame_size = 1024, .access_type = SND_PCM_ACCESS_MMAP_NONINTERLEAVED,
      .num_channels = 2, .number_of_frames = 1, .frame_size_direction =
          E_EXACT_CONFIG, .format = SND_PCM_FORMAT_S16_LE };

  snd_pcm_uframes_t buffer_size;

  /* Name of the PCM device, like @a plughw:0,0
   * @li The first number is the number of the soundcard
   * @li The second number is the number of the device.
   */
  char *pcm_name = "hw:1,0"; /* Rear headphones, obtained from audacity */

  uint8_t err = 0u;

  /* Allocate snd_pcm_hw_params_t structure on the stack.
   * @note we can also use @a snd_pcm_hw_params_malloc() to allocate the
   * memory, but we'll be responsible for free it when we finish using
   * @a snd_pcm_hw_params_free() */
  snd_pcm_hw_params_alloca(&hw_params);

  /* Create a handle and open a connection to a specified audio interface
   * This function receives as arguments:
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
  if (S_SUCCESS != err)
  {
    printf ("Error opening sound card\n");
    return S_ERROR;
  }

  /* Start setting HW parameters defined in headphones_hw_configuration */
  err = snd_pcm_hw_params_set_access (pcm_handle, hw_params,
                                      headphones_hw_configuration.access_type);
  if (S_SUCCESS != err)
  {
    printf ("Error setting access type\n");
    return S_ERROR;
  }

  err = snd_pcm_hw_params_set_format (pcm_handle, hw_params,
                                      headphones_hw_configuration.format);
  if (S_SUCCESS != err)
  {
    printf ("Error setting audio format type\n");
    return S_ERROR;
  }

  /* We'll set the sampling rate, and in case it's not supported by the sound
   * card, it will set the nearest sample rate supported,, the variable
   * @a headphones_hw_configuration.exact_sample_rate will have the configured
   * sample rate */
  err = snd_pcm_hw_params_set_rate_near (
      pcm_handle, hw_params, &headphones_hw_configuration.exact_sample_rate,
      &headphones_hw_configuration.sample_rate_direction);
  if (S_SUCCESS != err)
  {
    printf ("Error setting sample rate\n");
    return S_ERROR;
  }
  if (headphones_hw_configuration.sample_rate
      != headphones_hw_configuration.exact_sample_rate)
  {
    printf ("Sample rate not supported, using = %d Hz",
            headphones_hw_configuration.exact_sample_rate);
  }

  err = snd_pcm_hw_params_set_channels (
      pcm_handle, hw_params, headphones_hw_configuration.num_channels);
  if (S_SUCCESS != err)
  {
    printf ("Error setting number of channels\n");
    return S_ERROR;
  }

  err = snd_pcm_hw_params_set_periods_near (
      pcm_handle, hw_params, &headphones_hw_configuration.frame_size,
      &headphones_hw_configuration.frame_size_direction);
  if (S_SUCCESS != err)
  {
    printf ("Error setting number of periods\n");
    return S_ERROR;
  }

  /* @note buffer_size is the approximate target buffer size in frames */
  buffer_size = (headphones_hw_configuration.frame_size
      * headphones_hw_configuration.number_of_frames) >> 2;
  err = snd_pcm_hw_params_set_buffer_size_near (pcm_handle, hw_params,
                                                &buffer_size);
  if (S_SUCCESS != err)
  {
    printf ("Error setting number buffer size\n");
    return S_ERROR;
  }

  /* Apply the configuration to the sound card */
  err = snd_pcm_hw_params (pcm_handle, hw_params);
  if (S_SUCCESS != err)
  {
    fprintf (stderr, "Error setting HW params.\n");
    return (-1);
  }

}

/*************** END OF FILE **************************************************/
