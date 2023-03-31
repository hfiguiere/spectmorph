// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smutils.hh"

namespace SpectMorph
{

class TimeInfo
{
public:
  double time_ms = 0;
  double ppq_pos = 0;
};

class TimeInfoGenerator
{
  double   m_tempo = 120;
  double   m_ppq_pos = 0;
  double   m_last_block_ppq_pos = 0;
  double   m_mix_freq = 0;
  double   m_max_ppq_pos = 0;
  double   m_max_time_ms = 0;

  uint64   m_audio_time_stamp = 0;
  uint     m_n_samples = 0;
public:
  TimeInfoGenerator (double mix_freq);

  void start_block (uint64 audio_time_stamp, uint n_samples, double ppq_pos, double tempo);
  void update_time_stamp (uint64 audio_time_stamp);

  TimeInfo time_info (double offset_ms) const;
};

}
