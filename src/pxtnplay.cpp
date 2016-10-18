#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

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
"  -h --help        show this help and exit"                            "# bool <false>     <0>\n"
"  -v --version     show version and exit"                              "# bool <false>     <0>\n"
"  -q --quiet       no output"                                          "# bool <false>     <0>\n"
"  -f --config      config file"                                        "# string <>        <0>\n"
"\n"
"Play Option\n"
"  -B --buffer-size play buffer size [1-44100] [frame]"                 "# int  <441>       <1>\n"
"  -c --channels    channels         (1,2)"                             "# int  <2>         <1>\n"
"     --dummy       dummy output"                                       "# bool <false>     <1>\n"
"  -r --rate        sample rate      (44100,22050,11025) [KHz]"         "# int  <44100>     <1>\n"
"  -b --bit-rate    bit rate         (8,16) [bit]"                      "# int  <16>        <1>\n"
"\n"
"ALSA Options\n"
"  -d --device      target alsa device"                                 "# string <default> <1>\n"
"\n"
"Pxtone Options\n"
"  -l --loop        enable loop"                                        "# bool <false>     <1>\n"
"     --fadein      enable fade in   (0-10000) [ms]"                    "# int  <0>         <1>\n"
"     --fadeout     enable fade out  (0-10000) [ms]"                    "# int  <0>         <1>\n"
"  -V --volume      change volume    (0-100) [%]"                       "# int  <100>       <1>\n"
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
bool dummy_play(option::ppOption &opt, pxtoneVomit &p_vomit);

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
  // enable quiet option
  {
    bool quiet = false;
    if (opt.get("quiet", quiet) == true && quiet == true) {
      std::cout.setstate(std::ios_base::failbit);
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
      dump_error(ppErrPxtoneReadAsPxtone);
      dump_pxtone_error(p_vomit.get_last_error());
      return false;
    }

    // read pxtone file info
    std::cout << "------------- music info -------------" << std::endl;
    std::cout << "Title: " << util::MS932toUTF8(p_vomit.get_title())
              << std::endl;
    std::cout << "  Comment: " << util::MS932toUTF8(p_vomit.get_comment())
              << std::endl;

    s32 beat_num, beat_clock, meas_num;
    f32 beat_tempo;
    p_vomit.get_info(&beat_num, &beat_tempo, &beat_clock, &meas_num);

    std::cout << "  beat: " << beat_num << std::endl;
    std::cout << "  beat tempo: " << beat_tempo << std::endl;
    std::cout << "  beat clock: " << beat_clock << std::endl;
    std::cout << "  measure: " << meas_num << std::endl;

    {
      // config
      bool loop = false;
      unsigned long volume, channels, rate, bitrate;

      opt.get("loop", loop);
      opt.get("volume", volume);
      opt.get("channels", channels);
      opt.get("rate", rate);
      opt.get("bit-rate", bitrate);

      if (!p_vomit.set_loop(loop)) {
        dump_error(ppErrPxtoneSetLoop);
        dump_pxtone_error(p_vomit.get_last_error());
        return false;
      }
      if (!p_vomit.set_volume(volume / 100.0f)) {
        dump_error(ppErrPxtoneSetVolume);
        dump_pxtone_error(p_vomit.get_last_error());
        return false;
      }
      if (!p_vomit.set_quality(channels, rate, bitrate)) {
        dump_error(ppErrPxtoneSetQuality);
        dump_pxtone_error(p_vomit.get_last_error());
        return false;
      }
    }

    // vomit play
    {
      bool dummy;
      opt.get("dummy", dummy);
      if (dummy) {
        return dummy_play(opt, p_vomit);
      } else {
        return alsa_play(opt, p_vomit);
      }
    }
  }

  return true;
}

void show_player_info(option::ppOption &opt)
{
  // get config
  unsigned long channels, rate, bitrate, buffersize;
  opt.get("channels", channels);
  opt.get("rate", rate);
  opt.get("bit-rate", bitrate);
  opt.get("buffer-size", buffersize);

  // clang-format off
  std::cout << "----------- player config ------------"         << std::endl;
  std::cout << "  Quality: " << rate << "Hz / "
		        << (bitrate  == 8 ? " 8"     : "16"    ) << "bit / "
            << (channels == 1 ? "  mono" : "stereo")            << std::endl;
  std::cout << "  Buffer size: " << std::right << std::setw(15)
            << (buffersize * channels * bitrate / 8) << " byte" << std::endl;
  std::cout << "--------------------------------------"         << std::endl;
	// clang-formant on
}

void show_play_time(double span)
{
  static size_t count = 0;
  std::cout << "Time: " << util::printAsTime(count * span) << "\r";
  count++;
}

bool alsa_play(option::ppOption &opt, pxtoneVomit &p_vomit)
{
  int err;
  snd_pcm_t *pv_h;
  snd_pcm_hw_params_t *hw_params;

  // get config
  snd_pcm_format_t hw_format;
  unsigned long channels, rate, bitrate, buffersize;
  std::string device;
  opt.get("channels", channels);
  opt.get("rate", rate);
  opt.get("bit-rate", bitrate);
  opt.get("buffer-size", buffersize);
  opt.get("device", device);

  if (bitrate == 8) {
    hw_format = SND_PCM_FORMAT_U8;
  } else {
    hw_format = SND_PCM_FORMAT_S16_LE;
  }

  if ((err = snd_pcm_open(&pv_h, device.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) <
      0) {
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

  if ((err = snd_pcm_hw_params_set_channels(pv_h, hw_params, channels)) < 0) {
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
  unsigned long framesize = bitrate / 8 * channels;
  std::vector<std::uint8_t> buf((size_t)framesize * buffersize);

  show_player_info(opt);

  // play

  if (!p_vomit.Start(0, 0)) {
    dump_error(ppErrPxtoneStartFailure);
    dump_pxtone_error(p_vomit.get_last_error());
    return false;
  }

  double vomit_span = (double)buffersize / rate;
  while (p_vomit.vomit(buf.data(), buf.size())) {
    show_play_time(vomit_span);
    if ((err = snd_pcm_writei(pv_h, buf.data(), buffersize)) != buffersize) {
      dump_error(ppErrAlsaWriteInterface);
      dump_alsa_error(snd_strerror(err));
      snd_pcm_close(pv_h);
      return false;
    }
  }
  std::cout << std::endl;

  snd_pcm_close(pv_h);
  return true;
}
bool dummy_play(option::ppOption &opt, pxtoneVomit &p_vomit)
{
  // get config
  unsigned long channels, rate, bitrate, buffersize;
  opt.get("channels", channels);
  opt.get("rate", rate);
  opt.get("bit-rate", bitrate);
  opt.get("buffer-size", buffersize);

  // create buffer
  unsigned long framesize = bitrate / 8 * channels;
  std::vector<std::uint8_t> buf((size_t)framesize * buffersize);

  show_player_info(opt);

  // play
  if (!p_vomit.Start(0, 0)) {
    dump_error(ppErrPxtoneStartFailure);
    dump_pxtone_error(p_vomit.get_last_error());
    return false;
  }

  double vomit_span = (double)buffersize / rate;
  while (p_vomit.vomit(buf.data(), buf.size())) {
    show_play_time(vomit_span);
  }
  std::cout << std::endl;

  return true;
}
}
