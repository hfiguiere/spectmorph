// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LINEEDIT_HH
#define SPECTMORPH_LINEEDIT_HH

#include "smdrawutils.hh"
#include "smmath.hh"

namespace SpectMorph
{

struct LineEdit : public Widget
{
protected:
  std::string m_text;
  bool highlight = false;

public:
  void
  set_text (const std::string& new_text)
  {
    m_text = new_text;
  }
  std::string
  text() const
  {
    return m_text;
  }
  LineEdit (Widget *parent)
    : Widget (parent)
  {
    m_text = "/tmp/foo.smplan";
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double space = 2;
    Color fill_color;
    if (highlight)
      fill_color = ThemeColor::MENU_BG;

    Color text_color (1, 1, 1);
    Color frame_color = ThemeColor::FRAME;
    if (!recursive_enabled())
      {
        text_color = text_color.darker();
        frame_color = frame_color.darker();
      }

    du.round_box (0, space, width, height - 2 * space, 1, 5, frame_color, fill_color);

    du.set_color (text_color);
    du.text (m_text, 10, 0, width - 10, height);
    double w_ = du.text_width ("_");
    double tw = du.text_width ("_" + m_text + "_") - 2 * w_; /* also count spaces at start/end */
    du.round_box (10 + tw + 1, space * 2, w_, height - 4 * space, 1, 0, Color::null(), ThemeColor::SLIDER /* FIXME */ );
  }
  bool
  is_control (uint32 u)
  {
    return (u <= 0x1F) || (u >= 0x7F && u <= 0x9f);
  }
  static std::string
  utf8_from_unicode (const std::vector<uint32>& unicode)
  {
    std::string utf8 = "";
    for (auto c : unicode)
      {
        char buffer[8] = { 0, };
        g_unichar_to_utf8 (c, buffer);
        utf8 += buffer;
      }
    return utf8;
  }
  static std::vector<uint32>
  utf8_to_unicode (const std::string& utf8)
  {
    gunichar *uc = g_utf8_to_ucs4 (utf8.c_str(), -1, NULL, NULL, NULL);
    std::vector<uint32> chars;
    if (uc)
      {
        for (size_t i = 0; uc[i]; i++)
          chars.push_back (uc[i]);
      }
    g_free (uc);
    return chars;
  }
  virtual void
  key_press_event (const PuglEventKey& key_event) override
  {
    if (key_event.filter)
      {
        /* multi key sequence -> ignore */
        return;
      }
    if (!is_control (key_event.character) && key_event.utf8[0])
      {
        m_text += (const char *) key_event.utf8;
      }
    else if (key_event.character == PUGL_CHAR_BACKSPACE && !m_text.empty())
      {
        std::vector<uint32> chars = utf8_to_unicode (m_text);
        if (chars.size())
          chars.pop_back();
        m_text = utf8_from_unicode (chars);
      }
  }
  void
  enter_event() override
  {
    highlight = true;
  }
  void
  leave_event() override
  {
    highlight = false;
  }
};

}

#endif
