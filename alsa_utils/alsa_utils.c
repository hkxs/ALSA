/*******************************************************************************
 * @file      alsa_utils.c
 *
 * @brief      This module have common functions for ALSA
 *
 * This module should contain common functions used for different ALSA programs
 * this could be:
 *      @li HW Configuration
 *      @li SW Configuration
 *      @li Tone Generators
 *      @li Filters
 *      @li etc.
 *
 * @note Link using -lasound -lm
 *
 * @author      hkxs
 *
 *
 * MIT License
 *
 * Copyright (c) 2020 hkxs
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
 ------------------------------------------------------------------------------*/
#include "alsa_utils.h"

/*------------------------------------------------------------------------------
 * Module Preprocessor Constants
 ------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Module Typedefs
 ------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Function Prototypes
 ------------------------------------------------------------------------------*/
/******************************************************************************
 * @fn int8_t configure_hw(snd_pcm_t*, hw_configuration*)
 *
 * @brief Configure the HW of the sound card
 *
 * Set HW parameters defined in hw_configuration, part of this can
 * be done also with @a snd_pcm_set_params but I prefer to do it manually to
 * learn a little more of the APIs ¯\_(ツ)_/¯
 *
 * @param[in] *sound_card_handle     pointer to the handle of the sound card
 * @param[in] *hw_config             pointer to desired hw configuration
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
  buffer_size = (hw_config->period_size*hw_config->periods);
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

/******************************************************************************
 *
 * @fn int8_t generate_sin (int16_t*, uint16_t, uint16_t, uint32_t)
 *
 * @brief Generate a sine wave
 *
 * Generate a sine wave with the predefined frequency and it will returned in
 * Q14
 *
 * @note This function is supposed to be used in interleaved mode, for this
 *       reason the samples are stored directly in interleaved mode, e.g.
 *       1. data[0] = val1, data for channel 1
 *       2. data[1] = val1, data for channel 2
 *       3. data[2] = val2, data for channel 1
 *       4. data[3] = val2, data for channel 2
 *
 * @param[out] *data    Buffer to store the generated wave
 * @param       f       Frequency of the sine wave
 * @param       fs      Sampling frequency
 * @param       data_length     Size of the data buffer
 *
 * @return int8_t
 *
 ******************************************************************************/
int8_t generate_sin (int16_t *data, uint16_t f, uint16_t fs,
                     uint32_t data_length)
{

  float sin_val;
  uint8_t err = S_ERROR;

  if ( NULL!=data )
  {
    for (uint32_t n = 0; n<data_length; n++)
    {
      sin_val = sin (2*M_PI*n*f/fs);
      data[2*n] = (uint16_t)(sin_val*Q_14);
      data[2*n+1] = data[2*n];
    }
    err = S_SUCCESS;
  }

  return err;
}
/*-------------- END OF FILE -------------------------------------------------*/
