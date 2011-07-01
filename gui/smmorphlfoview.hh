/*
 * Copyright (C) 2011 Stefan Westerfeld
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

#ifndef SPECTMORPH_MORPH_LFO_VIEW_HH
#define SPECTMORPH_MORPH_LFO_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphlfo.hh"
#include "smcomboboxoperator.hh"

namespace SpectMorph
{

class MorphLFOView : public MorphOperatorView
{
protected:
  MorphLFO                        *morph_lfo;
  Gtk::Table                       table;

  OperatorFilter                  *operator_filter;

  Gtk::Label                       wave_type_label;
  Gtk::ComboBoxText                wave_type_combobox;

  void on_wave_type_changed();
public:
  MorphLFOView (MorphLFO *op, MorphPlanWindow *morph_plan_window);
  ~MorphLFOView();
};

}

#endif
