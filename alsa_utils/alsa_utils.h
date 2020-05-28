/*******************************************************************************
 * @file        alsa_utils.h
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
 * @note
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
 -----------------------------------------------------------------------------*/
#include <alsa/asoundlib.h>
#include <math.h>

/*------------------------------------------------------------------------------
 * Preprocessor Constants
 -----------------------------------------------------------------------------*/
#ifndef _ALSA_UTILS_
#define _ALSA_UTILS_

#define S_SUCCESS               (0x00) /**< used to return success */
#define S_ERROR                 (0x01) /**< used to return error */
#define PCM_OPEN_STANDARD_MODE  (0x00) /**< define standard mode */
#define Q_14                    (1<<14)/**< Used to convert from float to Q14*/

/*------------------------------------------------------------------------------
 * Configuration Constants
 -----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Typedefs
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
int8_t generate_sin (int16_t *data, uint16_t f, uint16_t fs,
                     uint32_t data_length);
#endif
/*-------------- END OF FILE -------------------------------------------------*/
