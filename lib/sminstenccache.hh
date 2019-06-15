// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INSTENCCACHE_HH
#define SPECTMORPH_INSTENCCACHE_HH

#include "smwavdata.hh"
#include "smencoder.hh"
#include "sminstrument.hh"

#include <mutex>
#include <regex>

namespace SpectMorph
{

class InstEncCache
{
  struct CacheData
  {
    std::string                version;
    std::vector<unsigned char> data;
  };

  std::map<std::string, CacheData> cache;
  std::mutex                       cache_mutex;
  const std::regex                 cache_file_re;

  void        cache_save (const std::string& key);
  void        cache_try_load (const std::string& key, const std::string& need_version);

  InstEncCache(); // Singleton -> private constructor
public:
  class Group
  {
  public:
    std::string id;
  };

  Audio      *encode (Group *group, const WavData& wav_data, const std::string& wav_data_hash,
                      int midi_note, int iclipstart, int iclipend, Instrument::EncoderConfig& cfg,
                      const std::function<bool()>& kill_function);
  void        clear();
  void        delete_old_files();
  Group      *create_group();

  static InstEncCache *the(); // Singleton
};

}

#endif
