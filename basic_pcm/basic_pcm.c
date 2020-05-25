/*******************************************************************************
 * @file          basic_pcm.c
 * @brief         Simple client for the ALSA playback
 *
 * Simple client for the ALSA playback
 *
 * @note          Link using -lasound and -lm, based on the HOWTO
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
#include <math.h>

/*------------------------------------------------------------------------------
 * Module Preprocessor Constants
 -----------------------------------------------------------------------------*/
#define S_SUCCESS               (0x00) /**< used to return success*/
#define S_ERROR                 (0x01)/**< used to return error*/
#define PCM_OPEN_STANDARD_MODE  (0x00)/**< define standard mode*/

/*------------------------------------------------------------------------------
 * Module Typedefs
 -----------------------------------------------------------------------------*/
/** This enum is used for configuring sample rate and number of periods
 * @todo I need to investigate more how this parameter affects the parameters
 *       selecting -1,0 or 1 doesn't seems to differ when I'm selecting
 *       unsupported sample rates (supported samples: 44100 48000 96000 192000):
 *       @li Test 1, sub_unit_direction = 0:
 *              - 45k => 44.1k
 *              - 47k => 48k
 *              - 48k => 48k
 *              - 60k => 48k
 *              - 90k => 96k
 *       @li Test 2, sub_unit_direction = -1:
 *              - 45k => 44.1k
 *              - 47k => 48k
 *              - 48k => 48k
 *              - 60k => 48k
 *              - 90k => 96k
 *       @li Test 3, sub_unit_direction = 1:
 *              - 45k => 44.1k
 *              - 47k => 48k
 *              - 48k => 48k
 *              - 60k => 48k
 *              - 90k => 96k
 * The sample rate doesn't seems to be affected by that parameter*/
typedef enum
{
  E_EXACT_CONFIG = 0, /**< exact_rate == rate --> dir = 0 */
  E_SMALLER_CONFIG = -1, /**< exact_rate < rate  --> dir = -1 */
  E_BIGGER_CONFIG = 1 /**< exact_rate > rate  --> dir = 1 */

} sub_unit_direction;

/** Structure used to setup the desired hw configuration */
typedef struct
{
  uint32_t sample_rate; /**< Desired/Supported sample rate*/

  sub_unit_direction sample_rate_direction; /**< type of strategy used for set
   unsupported sampling rates, see @ref sub_unit_direction*/

  sub_unit_direction frame_size_direction;/**< type of strategy used for set
   unsupported framse size, see @ref sub_unit_direction*/

  snd_pcm_uframes_t period_size; /**< control the PCM interrupt */

  uint32_t periods; /**< frequency to update the status */

  snd_pcm_access_t access_type; /**< Type of access mode see
   @ref configure_hw (@b snd_pcm_hw_params_set_access) */

  uint32_t num_channels; /**< Number of channels to use */
  snd_pcm_format_t format; /**< Audio format to be used, seed@ref configure_hw
   @b snd_pcm_hw_params_set_format*/
} hw_configuration;

/*------------------------------------------------------------------------------
 * Function Prototypes
 -----------------------------------------------------------------------------*/
int8_t configure_hw (snd_pcm_t *sound_card_handle, hw_configuration *hw_config);

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
  int16_t *noise;
  uint32_t noise_size;

  printf ("Generating random noise\n");
  noise_size = hw_configuration.period_size*hw_configuration.periods;
  noise = (int16_t*)malloc (sizeof(noise)*noise_size);

  if ( NULL==noise )
  {
    printf ("Error allocating memory for the audio signal\n");
    snd_pcm_close (pcm_handle);

    return S_ERROR;
  }

  for (uint32_t i = 0u; i<noise_size; i++)
  {
    noise[i] = (int16_t)random ();
  }

  printf ("Sending data to sound card\n");

  /** @b snd_pcm_writei With everything set we can start writing data the API
   *  is different depending of the access_type:
   * @li snd_pcm_writei for SND_PCM_ACCESS_MMAP_INTERLEAVED
   * @li snd_pcm_writen for SND_PCM_ACCESS_MMAP_NONINTERLEAVED */
  for (uint8_t i = 0u; i<10; i++)
  {
    err = snd_pcm_writei (pcm_handle, noise, hw_configuration.period_size);

    /* if we fail we try to recover the stream state*/
    if ( S_SUCCESS>err )
    {
      err = snd_pcm_recover (pcm_handle, err, 1);
    }

    /* if we fail from recovery we suspend everything*/
    if ( S_SUCCESS>err )
    {
      printf ("Error writing data to the sound card\n");
      free (noise);
      snd_pcm_close (pcm_handle);

      return S_ERROR;
    }
  }

  free (noise);
  /* close the sound card */
  snd_pcm_close (pcm_handle);

  return S_SUCCESS;
}

/******************************************************************************
 * @fn int8_t configure_hw(snd_pcm_t*, hw_configuration*)
 *
 * @brief Configure the HW of the sound card
 *
 * Set HW parameters defined in hw_configuration, part of this can
 * be done also with @a snd_pcm_set_params but I prefer to do it manually to
 * learn a little more of the APIs ¯\_(ツ)_/¯
 *
 * @param[in] sound_card_handle     pointer to the handle of the sound card
 * @param[in] hw_config             pointer to desired hw configuration
 *
 * @return int8_t @a S_SUCCESS in case of success, @a S_ERROR otherwise
 *
 ******************************************************************************/
int8_t configure_hw (snd_pcm_t *sound_card_handle, hw_configuration *hw_config)
{
  int8_t err;
  snd_pcm_uframes_t buffer_size;

  /* This structure contains information about    */
  /* the hardware and can be used to specify the  */
  /* configuration for the PCM stream. */
  snd_pcm_hw_params_t *hw_params;

  /**  @b snd_pcm_hw_params_alloca Allocate snd_pcm_hw_params_t structure
   * on the stack.W e can also use @a snd_pcm_hw_params_malloc() to allocate the
   * memory, but we'll be responsible for free it when we finish using
   * @a snd_pcm_hw_params_free() */
  snd_pcm_hw_params_alloca(&hw_params);

  /** @b snd_pcm_hw_params_any Read all the hardware configuration for the
   * sound card before setting the configuration we want*/
  err = snd_pcm_hw_params_any (sound_card_handle, hw_params);
  if ( S_SUCCESS>err )
  {
    printf ("configure_hw Error: getting HW configuration\n");
    return S_ERROR;
  }

  /** @b snd_pcm_hw_params_set_access Set the type of transfer mode, there are
   *  basically two kinds:
   * 1. Regular: using direct write functions
   * 2. Mmap: writing to a memory pointer,
   * and this two tyoes can be divided in:
   * 1. Interleaved: each frame in the buffer contains the consecutive sample
   *    data for the channels.
   * 2. NonInterleaved: the buffer contains alternating words of sample data
   *    for every channel
   * 3. SND_PCM_ACCESS_MMAP_COMPLEX when the access doesn't fit to interleaved
   *   and non-interleaved ring buffer organization  */
  err = snd_pcm_hw_params_set_access (sound_card_handle, hw_params,
                                      hw_config->access_type);
  if ( S_SUCCESS>err )
  {
    printf ("configure_hw Error: setting access type, Err = %d\n", err);
    return S_ERROR;
  }

  err = snd_pcm_hw_params_set_format (sound_card_handle, hw_params,
                                      hw_config->format);
  if ( S_SUCCESS>err )
  {
    printf ("configure_hw Error: setting audio format type, Err = %d\n", err);
    return S_ERROR;
  }

  /** @b snd_pcm_hw_params_set_rate_near We'll set the sampling rate (in Hz).
   *  In case it's not supported by the sound card, it will set the nearest
   *  sample rate supported,, the variable @a hw_configuration.exact_sample_rate
   *   will have the configured sample rate */
  uint32_t desired_sample_rate;
  desired_sample_rate = hw_config->sample_rate;
  err = snd_pcm_hw_params_set_rate_near (sound_card_handle, hw_params,
                                         &hw_config->sample_rate,
                                         &hw_config->sample_rate_direction);
  if ( S_SUCCESS>err )
  {
    printf ("configure_hw Error: setting sample rate, Err = %d\n", err);
    return S_ERROR;
  }
  if ( hw_config->sample_rate!=desired_sample_rate )
  {
    printf ("configure_hw Warning: rate %d not supported, using = %d Hz\n",
            desired_sample_rate, hw_config->sample_rate);
  }

  err = snd_pcm_hw_params_set_channels (sound_card_handle, hw_params,
                                        hw_config->num_channels);
  if ( S_SUCCESS>err )
  {
    printf ("configure_hw Error: setting number of channels, Err = %d\n", err);
    return S_ERROR;
  }

  err = snd_pcm_hw_params_set_periods_near (sound_card_handle, hw_params,
                                            &hw_config->periods,
                                            &hw_config->frame_size_direction);
  if ( S_SUCCESS>err )
  {
    printf ("configure_hw Error: setting number of periods\n");
    return S_ERROR;
  }

  /** @b snd_pcm_hw_params_set_buffer_size_near set the size of the buffer (in
   * bytes). This is calculated using the period_size and period
   * @f$  buffer\_size = period\_size*period @f$ where
   * @li @a period: number of divisions of the ring buffer
   * @li @a period_size: this control when the PCM interrupt is generated,
   *         e.g. 44.1Khz, and the period_size to 4410 frames the interrupt will
   *         be generated every 100ms
   *
   * The buffer size  is also used to determine the latency:
   * @f$ latency = \frac{period\_size*period}{sample\_rate*frame} =
   * \frac{buffer\_size}{sample\_rate*frame} @f$
   * where:
   *
   * @li frame: number of bytes required for all the samples for all the
   * channels, e.g.:
   * @li 1 frame of a Stereo 48khz 16bit PCM stream is 4 bytes.
   * @li 1 frame of a 5.1 48khz 16bit PCM stream is 12 bytes.
   *  */
  buffer_size = (hw_config->period_size*hw_config->periods)>>2;
  err = snd_pcm_hw_params_set_buffer_size_near (sound_card_handle, hw_params,
                                                &buffer_size);
  if ( S_SUCCESS>err )
  {
    printf ("configure_hw Error: setting number buffer size, Err = %d\n", err);
    return S_ERROR;
  }

  /** @b snd_pcm_hw_params Apply the configuration to the sound card, on success
   * it will set the sound card to  SND_PCM_STATE_SETUP state and will call
   * the function @a snd_pcm_prepare() automatically */
  err = snd_pcm_hw_params (sound_card_handle, hw_params);
  if ( S_SUCCESS>err )
  {
    printf ("configure_hw Error: setting HW params, Err = %d\n", err);
    return S_ERROR;
  }

  printf ("HW configuration successful\n");
  return S_SUCCESS;
}
/*-------------- END OF FILE -------------------------------------------------*/
