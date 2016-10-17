#include <cstdlib>
#include <fstream>
#include <iostream>

#include <pxtnService.h>
#include <pxtoneVomit.h>

#include <alsa/asoundlib.h>

#include "config.h"
#include "error.h"
#include "option.h"
#include "pxtnplay.h"
#include "util.h"

namespace
{
using std::cout;
using std::cerr;
using std::endl;

// clang-format off
const char help_string[] =
/*---------------------------------------------------------------------- # type <default> <from config> */
"Usage: pxtnplay [options] FILE\n"
"Play Pxtone Colage music files (.ptcop) in command line.\n"
"\n"
"General Options\n"
"  -h --help        show this help and exit"                            "# bool <false> <0>\n"
"  -v --version     show version and exit"                              "# bool <false> <0>\n"
"  -q --quiet       no output"                                          "# bool <false> <0>\n"
"  -f --config      config file"                                        "# string <>    <0>\n"
"\n"
"Play Option\n"
"  -B --buffer-size play buffer size [frame]"                           "# int  <441>   <1>\n"
"  -c --channels    channels         (1,2)"                             "# int  <2>     <1>\n"
"     --dummy       dummy output"                                       "# bool <false> <1>\n"
"  -r --rate        sample rate      (44100,22050,11025) [KHz]"         "# int  <44100> <1>\n"
"  -b --bit-rate    bit rate         (8,16) [bit]"                      "# int  <16>    <1>\n"
"\n"
"ALSA Options\n"
"  not implemented yet\n"
"\n"
"Pxtone Options\n"
"  -l --loop        enable loop"                                        "# bool <false> <1>\n"
"     --fadein      enable fade in   (0-10000) [ms]"                    "# int  <0>     <1>\n"
"     --fadeout     enable fade out  (0-10000) [ms]"                    "# int  <0>     <1>\n"
"  -V --volume      change volume    (0-100) [%]"                       "# int  <100>   <1>\n"
/*----------------------------------------------------------------------*/;
// clang-format on

void dump_help(pxtnplay::option::ppOption &opt)
{
  cout << opt.dumpHelp() << endl;
}
void dump_version()
{
  cout << "pxtnplay " PXTNPLAY_VERSION << endl;
  cout << "Pxtone Library: " << PXTONE_VERSION << endl;
  cout << "----------------------------------------------------\n"
          "  Copyright (c) 2016 STUDIO PIXEL\n"
          "  Copyright (c) 2016 stkchp\n"
          "  This software is released under the MIT License.\n"
          "    http://opensource.org/licenses/mit-license.php\n"
          "----------------------------------------------------\n"
       << endl;
}
void dump_error(const char *message) { cerr << message << endl; }
void dump_pxtone_error(const char *message)
{
  cerr << "  Pxtone Error: " << message << endl;
}
void dump_alsa_error(const char *message)
{
  cerr << "  ALSA Error: " << message << endl;
}
}

namespace pxtnplay
{
using namespace pxtnplay::error;

bool alsa_play(option::ppOption &opt, pxtoneVomit &p_vomit);

bool run_pxtnplay(int argc, char *argv[])
{
  // parse option
  option::ppOption opt;

  opt.parseHelp(help_string);

  // get option from command line
  if (!opt.parseCommand(argc, argv)) {
    // getopt already print error message. nothing todo
    return false;
  }

  // print help or version
  {
    bool ret = true;

    if (opt.get("help", ret) == true && ret == true) {
      dump_help(opt);
      return true;
    }

    if (opt.get("version", ret) == true && ret == true) {
      dump_version();
      return true;
    }
  }

  if (opt.dumpInputfile().size() == 0) {
    dump_error(ppErrNoInputFile);
    return false;
  }

#if 0
  // for debug
  opt.dumpOptions();
#endif

  // check file exists
  {
    std::ifstream f(opt.dumpInputfile());
    if (!f.good()) {
      dump_error(ppErrCantOpenFile);
      return false;
    }
  }

  pxwrDoc p_doc;

  if (!p_doc.Open_path(opt.dumpInputfile().c_str(), "rb")) {
    dump_error(ppErrCantReadFileIntoMemory);
    return false;
  }
  {
    pxtoneVomit p_vomit;
    if (!p_vomit.Init()) {
      dump_error(ppErrPxtoneInitFailure);
      return false;
    }

    if (!p_vomit.Read(&p_doc)) {
      dump_error(ppErrReadAsPxtone);
      dump_pxtone_error(p_vomit.get_last_error());
      return false;
    }

    // read pxtone file info
    std::cout << "Title: " << util::MS932toUTF8(p_vomit.get_title())
              << std::endl;
    std::cout << "Comment: " << util::MS932toUTF8(p_vomit.get_comment())
              << std::endl;

    s32 beat_num, beat_clock, meas_num;
    f32 beat_tempo;
    p_vomit.get_info(&beat_num, &beat_tempo, &beat_clock, &meas_num);

    std::cout << "    beat: " << beat_num << std::endl;
    std::cout << "    beat tempo: " << beat_tempo << std::endl;
    std::cout << "    beat clock: " << beat_clock << std::endl;
    std::cout << "    measure: " << meas_num << std::endl;

    {
      // config
      bool ret_b = false;
      unsigned long ret_i, channels, rate, bitrate;

      opt.get("loop", ret_b);
      p_vomit.set_loop(ret_b);
      opt.get("volume", ret_i);
      p_vomit.set_volume(100.0f / ret_i);
      opt.get("channels", channels);
      opt.get("rate", rate);
      opt.get("bitrate", bitrate);
      p_vomit.set_quality(channels, rate, bitrate);
    }

    // vomit play
    return alsa_play(opt, p_vomit);
  }

  return true;
}

bool alsa_play(option::ppOption &opt, pxtoneVomit &p_vomit)
{
  int i;
  int err;
  snd_pcm_t *pv_h;
  snd_pcm_hw_params_t *hw_params;

  // get config
  snd_pcm_format_t hw_format;
  unsigned long channels, rate, bitrate, buffersize;
  opt.get("channels", channels);
  opt.get("rate", rate);
  opt.get("bit-rate", bitrate);
  opt.get("buffer-size", buffersize);
  if (bitrate == 8) {
    hw_format = SND_PCM_FORMAT_S8;
  } else {
    hw_format = SND_PCM_FORMAT_S16_LE;
  }
  if ((err = snd_pcm_open(&pv_h, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    dump_error(ppErrAlsaOpenDevice);
    dump_alsa_error(snd_strerror(err));
    return false;
  }

  if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
    dump_error(ppErrAlsaAllocateParameter);
    dump_alsa_error(snd_strerror(err));
    return false;
  }

  if ((err = snd_pcm_hw_params_any(pv_h, hw_params)) < 0) {
    dump_error(ppErrAlsaAllocateParameter);
    dump_alsa_error(snd_strerror(err));
    return false;
  }

  if ((err = snd_pcm_hw_params_set_access(pv_h, hw_params,
                                          SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    dump_error(ppErrAlsaSetAccessType);
    dump_alsa_error(snd_strerror(err));
    return false;
  }

  if ((err = snd_pcm_hw_params_set_format(pv_h, hw_params, hw_format)) < 0) {
    dump_error(ppErrAlsaSetSampleFormat);
    dump_alsa_error(snd_strerror(err));
    return false;
  }

  unsigned int _rate = rate;
  if ((err = snd_pcm_hw_params_set_rate_near(pv_h, hw_params, &_rate, 0)) < 0) {
    dump_error(ppErrAlsaSetRateNear);
    dump_alsa_error(snd_strerror(err));
    return false;
  }

  if ((err = snd_pcm_hw_params_set_channels(pv_h, hw_params, 2)) < 0) {
    dump_error(ppErrAlsaSetChannels);
    dump_alsa_error(snd_strerror(err));
    return false;
  }

  if ((err = snd_pcm_hw_params(pv_h, hw_params)) < 0) {
    dump_error(ppErrAlsaSetParameters);
    dump_alsa_error(snd_strerror(err));
    return false;
  }

  snd_pcm_hw_params_free(hw_params);

  if ((err = snd_pcm_prepare(pv_h)) < 0) {
    dump_error(ppErrAlsaPrepareInterface);
    dump_alsa_error(snd_strerror(err));
    return false;
  }

  // create buffer
  int framesize = bitrate / 2 * channels;
  std::vector<std::uint8_t> buf((size_t)framesize * buffersize);

  // play
  while (p_vomit.vomit(buf.data(), buf.size())) {
    if ((err = snd_pcm_writei(pv_h, buf.data(), buffersize)) != buffersize) {
      dump_error(ppErrAlsaWriteInterface);
      dump_alsa_error(snd_strerror(err));
      snd_pcm_close(pv_h);
      return false;
    }
  }

  snd_pcm_close(pv_h);
  return true;
}
}
