// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_NOISE_DECODER_HH
#define SPECTMORPH_NOISE_DECODER_HH

#include "smrandom.hh"
#include "smnoisebandpartition.hh"
#include "smrtmemory.hh"

namespace SpectMorph
{

/**
 * \brief Decoder for the noise component (stochastic component) of the signal
 */
class NoiseDecoder
{
  double mix_freq;
  size_t block_size;

  float *cos_window;
  float *interpolated_spectrum;

  Random random_gen;
  NoiseBandPartition noise_band_partition;

  void apply_window (float *spectrum, float *fft_buffer);
  static float *make_k_array();

public:
  NoiseDecoder (double mix_freq,
                size_t block_size);
  ~NoiseDecoder();

  enum OutputMode { REPLACE, ADD, FFT_SPECTRUM, DEBUG_UNWINDOWED, DEBUG_NO_OUTPUT };

  void set_seed (int seed);
  void process (const RTAudioBlock& audio_block,
                float *samples,
                OutputMode output_mode = REPLACE,
                float portamento_stretch = 1.0);
  void precompute_tables();

  static size_t preferred_block_size (double mix_freq);
};

}

#endif
