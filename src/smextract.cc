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

#include <vector>
#include <stdio.h>
#include <bse/bsemain.h>
#include "stwafile.hh"
#include "frame.hh"
#include <assert.h>

using Stw::Codec::Frame;
using std::vector;

double
float_vector_delta (const vector<float>& a, const vector<float>& b)
{
  assert (a.size() == b.size());

  double d = 0;
  for (size_t i = 0; i < a.size(); i++)
    d += (a[i] - b[i]) * (a[i] - b[i]);
  return d;
}

struct TransientModel
{
  double start_volume;
  double end_volume;
  int start;
  int end;
};

void
transient_scale (const TransientModel& m, const vector<float>& in, vector<float>& out)
{
  out.resize (in.size());

  for (size_t n = 0; n < in.size(); n++)
    {
      if (n <= m.start)
        {
          out[n] = in[n] * m.start_volume;
        }
      else if (n < m.end)
        {
          double d = double (n - m.start) / double (m.end - m.start);
          out[n] = in[n] * ((1 - d) * m.start_volume + d * m.end_volume);
        }
      else // (n >= end)
        {
          out[n] = in[n] * m.end_volume;
        }
    }
}

void
optimize_transient_model (TransientModel& m, vector<float>& signal, const vector<float>& desired_signal)
{
  int nomod = 0;

  vector<float> trsignal;
  while (nomod < 3000)
    {
      transient_scale (m, signal, trsignal);
      double m_delta = float_vector_delta (trsignal, desired_signal);

      TransientModel new_m = m;
      new_m.start += g_random_int_range (-5, 5);
      new_m.end += g_random_int_range (-5, 5);
      new_m.start_volume += g_random_double_range (-0.001, 0.001);
      new_m.end_volume += g_random_double_range (-0.001, 0.001);

      if (new_m.end > signal.size())
        new_m.end = signal.size();
      if (new_m.start + 10 > new_m.end)
        new_m.start = new_m.end - 10;
      if (new_m.start < 0)
        new_m.start = 0;
      transient_scale (new_m, signal, trsignal);

      double new_m_delta = float_vector_delta (trsignal, desired_signal);

      if (new_m_delta < m_delta)
        {
          m = new_m;
          nomod = 0;
        }
      else
        {
          nomod++;
        }
    }
}

float
mag (float re, float im)
{
  return sqrt (re * re + im * im);
}

int
main (int argc, char **argv)
{
  bse_init_inprocess (&argc, &argv, NULL, NULL);

  SpectMorph::Audio audio;
  BseErrorType error = STWAFile::load (argv[1], audio);
  if (error)
    {
      fprintf (stderr, "can't load file: %s\n", argv[1]);
      exit (1);
    }

  int frame_size = audio.frame_size_ms * audio.mix_freq / 1000;

  if (strcmp (argv[2], "freq") == 0)
    {
      float freq_min = atof (argv[3]);
      float freq_max = atof (argv[4]);


      for (size_t i = 0; i < audio.contents.size(); i++)
        {
          Frame frame (audio.contents[i], frame_size);
          for (size_t n = 0; n < frame.freqs.size(); n++)
            {
              if (frame.freqs[n] > freq_min && frame.freqs[n] < freq_max)
                {
                  printf ("%zd %f %f\n", i, frame.freqs[n], frame.phases[n*2] /* optimistic */);
                }
            }
        }
    }
  else if (strcmp (argv[2], "spectrum") == 0)
    {
      int i = atoi (argv[3]);
      vector<double> spectrum;

      for (size_t n = 0; n < audio.contents[i].original_fft.size(); n += 2)
        {
          double re = audio.contents[i].original_fft[n];
          double im = audio.contents[i].original_fft[n + 1];
          spectrum.push_back (sqrt (re * re + im * im));
        }
      for (size_t n = 0; n < spectrum.size(); n++)
        {
          double s = 0;
          for (size_t r = 0; r < 1; r++)
            {
              if (n + r < spectrum.size())
                s = std::max (s, spectrum[n + r]);
              if (r < n)
                s = std::max (s, spectrum[n - r]);
            }
          printf ("%f %f\n", n * 0.5 * audio.mix_freq / spectrum.size(), s);
        }
    }
  else if (strcmp (argv[2], "frame") == 0)
    {
      int i = atoi (argv[3]);
      size_t frame_size = audio.contents[i].debug_samples.size();
      vector<float> sines (frame_size);

      for (size_t partial = 0; partial < audio.contents[i].freqs.size(); partial++)
        {
          double smag = audio.contents[i].phases[2 * partial];
          double cmag = audio.contents[i].phases[2 * partial + 1];
          double f    = audio.contents[i].freqs[partial];
          double phase = 0;

          // do a phase optimal reconstruction of that partial
          for (size_t n = 0; n < frame_size; n++)
            {
              phase += f / audio.mix_freq * 2.0 * M_PI;
              sines[n] += sin (phase) * smag;
              sines[n] += cos (phase) * cmag;
            }
        }
      
      // optimize transient parameter set
      vector<float> trsines;
      TransientModel m;
      m.start_volume = 1;
      m.end_volume = 1;
      m.start = 0;
      m.end = sines.size();
      optimize_transient_model (m, sines, audio.contents[i].debug_samples);
      transient_scale (m, sines, trsines);
      for (size_t n = 0; n < audio.contents[i].debug_samples.size(); n++)
        {
          double v = audio.contents[i].debug_samples[n];
          printf ("%zd %f %f %f\n", n, v, trsines[n], v - trsines[n]);
        }
    }
  else if (strcmp (argv[2], "frameparams") == 0)
    {
      int i = atoi (argv[3]);
      for(;;)
        {
          double maxm = 0;
          size_t maxp = 0;
          for (size_t partial = 0; partial < audio.contents[i].freqs.size(); partial++)
            {
              double m = mag (audio.contents[i].phases[partial * 2], audio.contents[i].phases[partial * 2 + 1]);
              if (m > maxm)
                {
                  maxm = m;
                  maxp = partial;
                }
            }
          if (maxm > 0)
            {
              printf ("%f Hz: %f\n", audio.contents[i].freqs[maxp], bse_db_from_factor (maxm, -200));
              audio.contents[i].phases[maxp * 2] = 0;
              audio.contents[i].phases[maxp * 2 + 1] = 0;
            }
          else
            {
              break;
            }
        }
    }
}
