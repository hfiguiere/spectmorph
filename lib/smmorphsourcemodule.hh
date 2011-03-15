/*
 * Copyright (C) 2010-2011 Stefan Westerfeld
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

#include "smmorphoperatormodule.hh"
#include "smwavset.hh"

namespace SpectMorph
{

class MorphSourceModule : public MorphOperatorModule
{
protected:
  WavSet wav_set;

  struct MySource : public LiveDecoderSource
  {
    void
    retrigger (int channel, float freq, int midi_velocity, float mix_freq)
    {
      g_printerr ("retrigger %d, %f, %f\n", channel, freq, mix_freq);
    }
    Audio* audio() { return NULL; }
    AudioBlock *audio_block (size_t index) { return NULL; }
  } my_source;
public:
  MorphSourceModule (MorphPlanVoice *voice);

  void set_config (MorphOperator *op);
  LiveDecoderSource *source();
};
}
