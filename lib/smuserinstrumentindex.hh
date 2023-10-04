// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_USER_INSTRUMENT_INDEX_HH
#define SPECTMORPH_USER_INSTRUMENT_INDEX_HH

#include "sminstrument.hh"

namespace SpectMorph
{

class UserInstrumentIndex
{
private:
  std::string user_instruments_dir;

public:
  UserInstrumentIndex();

  void create_instrument_dir (const std::string& bank);

  std::string filename (const std::string& bank, int number);
  std::string label (const std::string& bank, int number);

  int count (const std::string& bank);

  void remove_bank (const std::string& bank);
  void create_bank (const std::string& bank);
  std::vector<std::string> list_banks();

  Signal<> signal_banks_changed;
};

}

#endif
