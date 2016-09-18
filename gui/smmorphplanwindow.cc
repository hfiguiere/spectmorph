// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <assert.h>
#include <sys/time.h>

#include <vector>
#include <string>
#include <iostream>

#include <QMenuBar>
#include <QFileDialog>
#include <QTimer>

#include "smmain.hh"
#include "smmorphplan.hh"
#include "smmorphsource.hh"
#include "smmorphoutput.hh"
#include "smmorphlinear.hh"
#include "smmorphplanview.hh"
#include "smmorphplanwindow.hh"
#include "smrenameoperatordialog.hh"
#include "smmorphoperatorview.hh"
#include "smstdioout.hh"
#include "smmemout.hh"
#include "smhexstring.hh"
#include "smutils.hh"
#include "smmicroconf.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphPlanWindow::MorphPlanWindow (MorphPlanPtr morph_plan, const string& title) :
  win_title (title),
  m_morph_plan (morph_plan)
{
  /* actions ... */
  QAction *import_action = new QAction ("&Import...", this);
  QAction *export_action = new QAction ("&Export...", this);
  QAction *load_index_action = new QAction ("&Load Index...", this);

  connect (import_action, SIGNAL (triggered()), this, SLOT (on_file_import_clicked()));
  connect (export_action, SIGNAL (triggered()), this, SLOT (on_file_export_clicked()));
  connect (load_index_action, SIGNAL (triggered()), this, SLOT (on_load_index_clicked()));

  /* menus... */
  QMenuBar *menu_bar = menuBar();

  QMenu *file_menu = menu_bar->addMenu ("&File");
  QMenu *load_template_menu = file_menu->addMenu ("&Open Template");

  file_menu->addAction (import_action);
  file_menu->addAction (export_action);
  file_menu->addAction (load_index_action);

  QMenu *edit_menu = menu_bar->addMenu ("&Edit");
  QMenu *add_op_menu = edit_menu->addMenu ("&Add Operator");

  add_op_action (add_op_menu, "Source", "SpectMorph::MorphSource");
  add_op_action (add_op_menu, "Output", "SpectMorph::MorphOutput");
  add_op_action (add_op_menu, "Linear Morph", "SpectMorph::MorphLinear");
  add_op_action (add_op_menu, "Grid Morph", "SpectMorph::MorphGrid");
  add_op_action (add_op_menu, "LFO", "SpectMorph::MorphLFO");

  fill_template_menu (load_template_menu);

  /* central widget */
  morph_plan_view = new MorphPlanView (morph_plan.c_ptr(), this);
  setCentralWidget (morph_plan_view);
  update_window_title();

  connect (morph_plan_view, SIGNAL (view_widgets_changed()), this, SLOT (on_view_widgets_changed()));
}

void
MorphPlanWindow::set_filename (const string& filename)
{
  m_filename = filename;
  update_window_title();
}

void
MorphPlanWindow::update_window_title()
{
  if (m_filename != "")
    {
      QFileInfo fi (m_filename.c_str());
      setWindowTitle (fi.baseName() + (" - " + win_title).c_str());
    }
  else
    setWindowTitle (win_title.c_str());
}

void
MorphPlanWindow::add_op_action (QMenu *menu, const char *title, const char *type)
{
  QAction *action = new QAction (title, this);
  action->setData (type);

  menu->addAction (action);
  connect (action, SIGNAL (triggered()), this, SLOT (on_add_operator()));
}

void
MorphPlanWindow::fill_template_menu (QMenu *menu)
{
  MicroConf cfg (sm_get_install_dir (INSTALL_DIR_TEMPLATES) + "/index.smpindex");

  if (!cfg.open_ok())
    {
      return;
    }

  while (cfg.next())
    {
      string filename, title;

      if (cfg.command ("plan", filename, title))
        {
          QAction *action = new QAction (title.c_str(), this);
          action->setData (filename.c_str());
          menu->addAction (action);
          connect (action, SIGNAL (triggered()), this, SLOT (on_load_template()));
        }
    }
}

bool
MorphPlanWindow::load (const string& filename)
{
  GenericIn *in = StdioIn::open (filename);
  if (in)
    {
      m_morph_plan->load (in);
      delete in; // close file

      set_filename (filename);

      return true;
    }
  return false;
}

void
MorphPlanWindow::on_load_template()
{

  QAction *action = qobject_cast<QAction *>(sender());
  string filename = sm_get_install_dir (INSTALL_DIR_TEMPLATES) + "/" + string (qvariant_cast<QString> (action->data()).toLatin1().data());

  if (!load (filename))
    {
      QMessageBox::critical (this, "Error",
                             string_locale_printf ("Loading template failed, unable to open file '%s'.", filename.c_str()).c_str());
    }
}

void
MorphPlanWindow::on_file_import_clicked()
{
  QString file_name = QFileDialog::getOpenFileName (this, "Select SpectMorph plan to import", "", "SpectMorph plan files (*.smplan)");
  if (!file_name.isEmpty())
    {
      QByteArray file_name_local = QFile::encodeName (file_name);
      if (!load (file_name_local.data()))
        {
          QMessageBox::critical (this, "Error",
                                 string_locale_printf ("Import failed, unable to open file '%s'.", file_name_local.data()).c_str());
        }
    }
}

void
MorphPlanWindow::on_file_export_clicked()
{
  QString file_name = QFileDialog::getSaveFileName (this, "Select SpectMorph plan file", "", "SpectMorph plan files (*.smplan)");
  if (!file_name.isEmpty())
    {
      QByteArray file_name_local = QFile::encodeName (file_name);
      GenericOut *out = StdioOut::open (file_name_local.data());
      if (out)
        {
          m_morph_plan->save (out);
          delete out; // close file

          set_filename (file_name_local.data());
        }
      else
        {
          QMessageBox::critical (this, "Error",
                                 string_locale_printf ("Export failed, unable to open file '%s'.", file_name_local.data()).c_str());
        }
    }
}

void
MorphPlanWindow::on_load_index_clicked()
{
  QString file_name = QFileDialog::getOpenFileName (this, "Select SpectMorph index file", "", "SpectMorph index files (*.smindex)");

  if (!file_name.isEmpty())
    {
      QByteArray file_name_local = QFile::encodeName (file_name);
      m_morph_plan->load_index (file_name_local.data());
    }
}

void
MorphPlanWindow::on_add_operator()
{
  QAction *action = qobject_cast<QAction *>(sender());
  string type = qvariant_cast<QString> (action->data()).toLatin1().data();
  MorphOperator *op = MorphOperator::create (type, m_morph_plan.c_ptr());

  g_return_if_fail (op != NULL);

  m_morph_plan->add_operator (op, MorphPlan::ADD_POS_AUTO);
}

MorphOperator *
MorphPlanWindow::where (MorphOperator *op, const QPoint& pos)
{
  vector<int> start_position;
  int end_y = 0;

  MorphOperator *result = NULL;

  const vector<MorphOperatorView *> op_views = morph_plan_view->op_views();
  if (!op_views.empty())
    result = op_views[0]->op();

  for (vector<MorphOperatorView *>::const_iterator vi = op_views.begin(); vi != op_views.end(); vi++)
    {
      MorphOperatorView *view = *vi;

      QPoint view_pos = view->mapToGlobal (QPoint (0, 0));

      if (view_pos.y() < pos.y())
        result = view->op();

      end_y = view_pos.y() + view->height();
    }

  if (pos.y() > end_y)  // below last operator?
    return NULL;
  else
    return result;
}

void
MorphPlanWindow::add_control_widget (QWidget *widget)
{
  morph_plan_view->add_control_widget (widget);
}

void
MorphPlanWindow::on_view_widgets_changed()
{
  QTimer::singleShot (0, this, SLOT (on_update_window_size()));
}

void
MorphPlanWindow::on_update_window_size()
{
  resize (minimumWidth(), minimumHeight());
}
