// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INST_EDIT_NOTE_HH
#define SPECTMORPH_INST_EDIT_NOTE_HH

#include "smcheckbox.hh"
#include "smparamlabel.hh"
#include "smbutton.hh"
#include "smshortcut.hh"
#include "smtoolbutton.hh"
#include "smcombobox.hh"
#include "smfixedgrid.hh"
#include "smscrollview.hh"

namespace SpectMorph
{

class NoteWidget : public Widget
{
  int first = 12;
  int cols = 13;
  int rows = 9;
  int step = 12;

  struct NoteRect
  {
    int  note;
    Rect rect;
  };
  std::vector<NoteRect> note_rects;

  std::string
  note_to_text (int i, bool verbose = false)
  {
    std::vector<std::string> note_name { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    if (!verbose)
      {
        /* short names */
        if (note_name [i % 12][1] == '#')
          return "";
        else if (i % 12)
          return string_printf ("%s", note_name[i % 12].c_str());
      }
    /* full note name */
    return string_printf ("%s%d", note_name[i % 12].c_str(), i / 12 - 2);
  }
  int mouse_note = -1;

public:
  NoteWidget (Widget *parent)
    : Widget (parent)
  {
  }
  void
  draw (const DrawEvent& devent) override
  {
    DrawUtils du (devent.cr);

    du.round_box (0, 0, width, height, 1, 5, Color::null(), Color (0.7, 0.7, 0.7));

    auto cr = devent.cr;

    note_rects.clear();
    for (int xpos = 0; xpos < cols; xpos++)
      {
        for (int ypos = 0; ypos < rows; ypos++)
          {
            double x = xpos * width / cols;
            double y = ypos * height / rows;
            int n = first + xpos + (rows - 1 - ypos) * step;
            note_rects.push_back ({n, Rect (x, y, width / cols, height / rows)});
            if (!note_to_text (n).empty())
              {
                du.set_color (Color (0.0, 0.0, 0.0));
                du.text (note_to_text (n), x, y, width / cols, height / rows, TextAlign::CENTER);
              }
            else
              {
                cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
                cairo_rectangle (cr, x, y, width / cols, height / rows);
                cairo_fill (cr);
              }
            if (n == mouse_note)
              {
                double xspace = width / cols / 10;
                double yspace = height / rows / 10;
                du.round_box (x + xspace, y + yspace, width / cols - 2 * xspace, height / rows - 2 * yspace, 1, 5, Color (0.8, 0.8, 0.8), Color (0.9, 0.9, 0.9));

                du.set_color (Color (0.5, 0.0, 0.0));
                du.text (note_to_text (n, true), x, y, width / cols, height / rows, TextAlign::CENTER);
              }
          }
      }

    du.set_color (Color (0.4, 0.4, 0.4));
    cairo_set_line_width (cr, 1);
    for (int r = 1; r < rows; r++)
      {
        double y = r * height / rows;
        cairo_move_to (cr, 0, y);
        cairo_line_to (cr, width, y);
        cairo_stroke (cr);
      }
    for (int c = 1; c < cols; c++)
      {
        double x = c * width / cols;
        cairo_move_to (cr, x, 0);
        cairo_line_to (cr, x, height);
        cairo_stroke (cr);
      }
    du.round_box (0, 0, width, height, 1, 5, Color (0.4, 0.4, 0.4));
  }
  void
  motion (double x, double y)
  {
    for (auto note_rect : note_rects)
      if (note_rect.rect.contains (x, y))
        {
          if (note_rect.note != mouse_note)
            {
              mouse_note = note_rect.note;
              update();
            }
        }
  }
};

class InstEditNote : public Window
{
  Window     *parent_window = nullptr;
public:
  InstEditNote (Window *window, Instrument *instrument) :
    Window (*window->event_loop(), "SpectMorph - Instrument Note", 524, 348, 0, false, window->native_window()),
    parent_window (window)
  {
    set_close_callback ([this]() {
      signal_closed();
      delete_later();
     });

    Shortcut *play_shortcut = new Shortcut (this, ' ');
    connect (play_shortcut->signal_activated, [this]() { signal_toggle_play(); });

    FixedGrid grid;
    grid.add_widget (new NoteWidget (this), 1, 1, width / 8 - 2, height / 8 - 2);

    show();
  }
  Signal<> signal_toggle_play;
  Signal<> signal_closed;
};

}

#endif