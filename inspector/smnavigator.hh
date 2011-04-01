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

#ifndef SPECTMORPH_NAVIGATOR_HH
#define SPECTMORPH_NAVIGATOR_HH

#include <gtkmm.h>

#include "smwavset.hh"

namespace SpectMorph {

class Navigator : public Gtk::Window
{
  SpectMorph::WavSet    wset;
  std::string           wset_filename;
  std::string           wset_active_text;
  bool                  wset_edit;

  std::string           smset_dir;
  Gtk::ComboBoxText     smset_combobox;
  Gtk::VBox             index_vbox;
  Gtk::ToggleButton     show_position_button;
  Gtk::ToggleButton     show_analysis_button;
  Gtk::Button           save_button;
  std::vector<float>    decoded_samples;

  struct ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
    ModelColumns()
    {
      add (col_note);
      add (col_channel);
      add (col_range);
      add (col_path);
      add (col_wave_nr);
    }
    Gtk::TreeModelColumn<int>           col_note;
    Gtk::TreeModelColumn<int>           col_channel;
    Gtk::TreeModelColumn<Glib::ustring> col_range;
    Gtk::TreeModelColumn<Glib::ustring> col_path;
    Gtk::TreeModelColumn<int>           col_wave_nr;
  };

  ModelColumns audio_chooser_cols;
  Glib::RefPtr<Gtk::ListStore>       ref_tree_model;
  Glib::RefPtr<Gtk::TreeSelection>   ref_tree_selection;
  Gtk::ScrolledWindow                tree_view_scrolled_window;
  Gtk::TreeView                      tree_view;

  Gtk::ToggleButton                  source_button;

  GslDataHandle                     *dhandle;
  Audio                             *audio;

public:
  sigc::signal<void> signal_dhandle_changed;
  sigc::signal<void> signal_show_position_changed;
  sigc::signal<void> signal_show_analysis_changed;

  Navigator (const std::string& filename);

  void on_combo_changed();
  void on_selection_changed();
  void on_show_position_changed();
  void on_show_analysis_changed();
  void on_save_clicked();
  void on_audio_edit();

  GslDataHandle *get_dhandle();
  Audio         *get_audio();
  bool           get_show_position();
  bool           get_show_analysis();
};

}

#endif
