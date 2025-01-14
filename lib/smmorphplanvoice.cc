// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphplanvoice.hh"
#include "smmorphoutputmodule.hh"
#include "smleakdebugger.hh"
#include "smmidisynth.hh"
#include <assert.h>
#include <map>

using namespace SpectMorph;
using std::vector;
using std::string;
using std::sort;
using std::map;
using std::max;

static LeakDebugger leak_debugger ("SpectMorph::MorphPlanVoice");

MorphPlanVoice::MorphPlanVoice (float mix_freq, MorphPlanSynth *synth) :
  m_control_input (MorphPlan::N_CONTROL_INPUTS),
  m_mix_freq (mix_freq),
  m_morph_plan_synth (synth)
{
  leak_debugger.add (this);
}

void
MorphPlanVoice::configure_modules()
{
  for (size_t i = 0; i < modules.size(); i++)
    modules[i].module->set_config (modules[i].config);
}

MorphPlanVoice::~MorphPlanVoice()
{
  leak_debugger.del (this);
}

MorphOutputModule *
MorphPlanVoice::output()
{
  return m_output;
}

MorphOperatorModule *
MorphPlanVoice::module (const MorphOperatorPtr& ptr)
{
  MorphOperator::PtrID ptr_id = ptr.ptr_id();

  for (size_t i = 0; i < modules.size(); i++)
    if (modules[i].ptr_id == ptr_id)
      return modules[i].module.get();

  return NULL;
}

void
MorphPlanVoice::full_update (MorphPlanSynth::FullUpdateVoice& full_update_voice)
{
  /* This will loose the original state information which means the audio
   * will not transition smoothely. However, this should only occur for plan
   * changes, not parameter updates.
   */

  // exchange old modules with new modules
  //  - avoids allocating any memory here (in audio thread)
  //  - avoids freeing any memory here (in audio thread), this is done later when the update structure is freed
  modules.swap (full_update_voice.new_modules);
  m_output = full_update_voice.output_module;

  // reconfigure modules
  configure_modules();
}

void
MorphPlanVoice::cheap_update (MorphPlanSynth::UpdateP update)
{
  g_return_if_fail (update->ops.size() == modules.size());

  // set new configs from update
  for (size_t i = 0; i < modules.size(); i++)
    {
      assert (modules[i].ptr_id == update->ops[i].ptr_id);
      modules[i].config = update->ops[i].config;
      assert (modules[i].config);
    }

  // reconfigure modules
  configure_modules();
}

double
MorphPlanVoice::control_input (double value, MorphOperator::ControlType ctype, MorphOperatorModule *module)
{
  switch (ctype)
    {
      case MorphOperator::CONTROL_GUI:      return value;
      case MorphOperator::CONTROL_SIGNAL_1: return m_control_input[0];
      case MorphOperator::CONTROL_SIGNAL_2: return m_control_input[1];
      case MorphOperator::CONTROL_SIGNAL_3: return m_control_input[2];
      case MorphOperator::CONTROL_SIGNAL_4: return m_control_input[3];
      case MorphOperator::CONTROL_VELOCITY: return m_velocity * 2 - 1; // for modulation, this has to be signed
      case MorphOperator::CONTROL_OP:       return module->value();
      default:                              g_assert_not_reached();
    }
}

void
MorphPlanVoice::set_control_input (int i, double value)
{
  assert (i >= 0 && i < MorphPlan::N_CONTROL_INPUTS);

  m_control_input[i] = value;
}

void
MorphPlanVoice::set_velocity (float velocity)
{
  m_velocity = velocity;
}

float
MorphPlanVoice::velocity() const
{
  return m_velocity;
}

float
MorphPlanVoice::mix_freq() const
{
  return m_mix_freq;
}

MorphPlanSynth *
MorphPlanVoice::morph_plan_synth() const
{
  return m_morph_plan_synth;
}

void
MorphPlanVoice::update_shared_state (const TimeInfo& time_info)
{
  for (size_t i = 0; i < modules.size(); i++)
    modules[i].module->update_shared_state (time_info);
}

void
MorphPlanVoice::reset_value (const TimeInfo& time_info)
{
  for (size_t i = 0; i < modules.size(); i++)
    modules[i].module->reset_value (time_info);
}

void
MorphPlanVoice::fill_notify_buffer (NotifyBuffer& buffer)
{
  VoiceOpValuesEvent::Voice voices[modules.size()];
  int n = 0;

  for (const MorphPlanSynth::OpModule& m : modules)
    {
      float notify_value;

      if (m.module->get_notify_value (notify_value))
        {
          voices[n].voice = (uintptr_t) this;
          voices[n].op    = m.ptr_id;
          voices[n].value = notify_value;
          n++;
        }
    }
  if (n)
    {
      buffer.write_int (VOICE_OP_VALUES_EVENT);
      buffer.write_seq (voices, n);
    }
}
