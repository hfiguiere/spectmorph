/*
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SPECTMORPH_IFFT_SYNTH_HH
#define SPECTMORPH_IFFT_SYNTH_HH

#include <sys/types.h>
#include <vector>

#include "smmath.hh"

namespace SpectMorph {

class IFFTSynthTable;

class IFFTSynth
{
  IFFTSynthTable    *table;

  int                zero_padding;
  size_t             block_size;
  double             mix_freq;
  double             freq256_factor;
  double             mag_norm;

  float             *fft_in;
  float             *fft_out;
  float             *win_scale;

  enum { 
    SIN_TABLE_SIZE = 4096,
    SIN_TABLE_MASK = 4095
  };

  static std::vector<float> sin_table;

public:
  enum WindowType { WIN_BLACKMAN_HARRIS_92, WIN_HANNING };
  enum OutputMode { REPLACE, ADD };

  IFFTSynth (size_t block_size, double mix_freq, WindowType win_type);
  ~IFFTSynth();

  void
  clear_partials()
  {
    zero_float_block (block_size, fft_in);
  }

  float*
  fft_buffer()
  {
    return fft_in;
  }

  inline void render_partial (double freq, double mag, double phase);
  void get_samples (float *samples, OutputMode output_mode = REPLACE);

  double quantized_freq (double freq);
};

struct IFFTSynthTable
{
  std::vector<float> win_trans;

  float             *win_scale;
};

inline void
IFFTSynth::render_partial (double mf_freq, double mag, double phase)
{
  const int range = 4;

  const int freq256 = sm_round_positive (mf_freq * freq256_factor);
  const int ibin = freq256 >> 8;
  float *sp = fft_in + 2 * (ibin - range);
  const float *wmag_p = &table->win_trans[(freq256 & 0xff) * (range * 2 + 1)];

  // adjust phase to get the same output like vector sin (smmath.hh)
  // phase_adjust = freq256 * (M_PI / 256.0) - M_PI / 2;
  const double phase_adjust = (freq256 - 128) * (M_PI / 256.0);

  const float nmag = mag * mag_norm;

  // rotation for initial phase; scaling for magnitude

  /* the following block computes sincos (-phase-phase_adjust) */
  const double inv_2pi = 1.0 / (2 * M_PI);
  double sarg = (phase + phase_adjust) * inv_2pi;

  int iarg = sm_round_positive (sarg * SIN_TABLE_SIZE);
  const float phase_rsmag = sin_table [iarg & SIN_TABLE_MASK] * nmag;
  iarg += SIN_TABLE_SIZE / 4;
  const float phase_rcmag = sin_table [iarg & SIN_TABLE_MASK] * nmag;

  /* compute FFT spectrum modifications */
  if (ibin > range && 2 * (ibin + range) < static_cast<int> (block_size))
    {
      for (int i = 0; i <= 2 * range; i++)
        {
          const float wmag = wmag_p[i];
          *sp++ += phase_rcmag * wmag;
          *sp++ += phase_rsmag * wmag;
        }
    }
  else
    {
      wmag_p += range; // allow negative addressing
      for (int i = -range; i <= range; i++)
        {
          const float wmag = wmag_p[i];
          if ((ibin + i) < 0)
            {
              fft_in[-(ibin + i) * 2] += phase_rcmag * wmag;
              fft_in[-(ibin + i) * 2 + 1] -= phase_rsmag * wmag;
            }
          else if ((ibin + i) == 0)
            {
              fft_in[0] += 2 * phase_rcmag * wmag;
            }
          else if (2 * (ibin + i) == static_cast<int> (block_size))
            {
              fft_in[1] += 2 * phase_rcmag * wmag;
            }
          else if (2 * (ibin + i) > static_cast<int> (block_size))
            {
              int p = block_size - (2 * (ibin + i) - block_size);

              fft_in[p] += phase_rcmag * wmag;
              fft_in[p + 1] -= phase_rsmag * wmag;
            }
          else // no corner case
            {
              fft_in[(ibin + i) * 2] += phase_rcmag * wmag;
              fft_in[(ibin + i) * 2 + 1] += phase_rsmag * wmag;
            }
        }
    }
}

}

#endif
