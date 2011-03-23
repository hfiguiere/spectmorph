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

#ifndef SPECTMORPH_STDIO_IN_HH
#define SPECTMORPH_STDIO_IN_HH

#include <string>
#include "smgenericin.hh"

namespace SpectMorph
{

class StdioIn : public GenericIn
{
  FILE        *file;
  std::string  filename;

  StdioIn (FILE *file, const std::string& filename);
  ~StdioIn();
public:
  static GenericIn* open (const std::string& filename);

  int get_byte();     // like fgetc
  int read (void *ptr, size_t size);
  bool skip (size_t size);
  unsigned char *mmap_mem (size_t& remaining);
  size_t get_pos();
  GenericIn *open_subfile (size_t pos, size_t len);
};

}

#endif /* SPECTMORPH_STDIO_IN_HH */
