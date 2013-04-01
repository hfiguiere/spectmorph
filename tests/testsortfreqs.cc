// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <sys/time.h>
#include <stdio.h>
#include <assert.h>

#include "smaudio.hh"
#include "smmain.hh"

using namespace SpectMorph;
using std::vector;

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

using namespace SpectMorph;

struct PData
{
  float freq;
  float mag;
  float phase;
};

float
something()
{
  return g_random_double_range (0, 0.1);
}

void
randomize_and_check (AudioBlock& block)
{
  vector<PData> partials;

  for (size_t i = 0; i < 30; i++)
    {
      PData pd;
      pd.freq = i * 440 + something();
      pd.mag = i * 0.1 + something();
      pd.phase = i * 0.001 + something();

      partials.push_back (pd);
    }
  vector<PData> partials_shuffle = partials;
  std::random_shuffle (partials_shuffle.begin(), partials_shuffle.end());

  assert (partials.size() == partials_shuffle.size());

  block.freqs.clear();
  block.mags.clear();
  block.phases.clear();

  for (size_t i = 0; i < partials.size(); i++)
    {
      block.freqs.push_back (partials_shuffle[i].freq);
      block.mags.push_back (partials_shuffle[i].mag);
      block.phases.push_back (partials_shuffle[i].phase);
    }

  AudioBlock check_block = block;
  check_block.sort_freqs();
  for (size_t i = 0; i < check_block.freqs.size(); i++)
    {
      assert (check_block.freqs[i] == partials[i].freq);
      assert (check_block.mags[i] == partials[i].mag);
      assert (check_block.phases[i] == partials[i].phase);
    }
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  AudioBlock block_a, block_b, block_c;

  randomize_and_check (block_a);
  randomize_and_check (block_b);

  const unsigned int runs = 1000000;

  // assignments are not cheap, so we measure them seperately and subtract the result
  double xstart = gettime();
  for (unsigned int i = 0; i < runs; i++)
    {
      block_c = block_a;
      block_c = block_b;
    }
  double xend = gettime();
  double xtime = xend - xstart;

  printf ("%.2f assign_blocks/sec\n", runs / 2 / xtime);

  double start = gettime();
  for (unsigned int i = 0; i < runs; i++)
    {
      block_c = block_a;
      block_c.sort_freqs();
      block_c = block_b;
      block_c.sort_freqs();
    }
  double end = gettime();
  printf ("%.2f sort_freqs/sec\n", runs / 2 / (end - start - xtime));
}
