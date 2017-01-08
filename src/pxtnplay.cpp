#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include <pxtnService.h>

#include <pxtnError.h>

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
// "  -f --config      config file"                                        "# string <>        <0>\n"
"\n"
"Play Option\n"
"  -B --buffer-size play buffer size (1-44100) [frame]"                 "# int  <441>       <1>\n"
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
// "     --fadeout     enable fade out  (0-10000) [ms]"                    "# int  <0>         <1>\n"
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
bool load_ptcop(pxtnService &pxtn, const char *path_src, pxtnERR &p_pxtn_err)
{
  bool b_ret = false;
  pxtnDescriptor desc;
  pxtnERR pxtn_err = pxtnERR_VOID;
  FILE *fp = NULL;
  int32_t event_num = 0;

  if (!(fp = fopen(path_src, "rb"))) goto term;
  if (!desc.set_file_r(fp)) goto term;

  pxtn_err = pxtn.read(&desc);
  if (pxtn_err != pxtnOK) goto term;
  pxtn_err = pxtn.tones_ready();
  if (pxtn_err != pxtnOK) goto term;

  b_ret = true;
term:

  if (fp) fclose(fp);
  if (!b_ret) pxtn.evels->Release();

  if (p_pxtn_err) p_pxtn_err = pxtn_err;

  return b_ret;
}
}

namespace pxtnplay
{
using namespace pxtnplay::error;

bool alsa_play(option::ppOption &opt, pxtnService &pxtn);
bool dummy_play(option::ppOption &opt, pxtnService &pxtn);

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

  {
    pxtnService pxtn;
    pxtnERR pxtn_err = pxtnERR_VOID;
    int32_t buf_size = 0;

    pxtn_err = pxtn.init();
    if (pxtn_err != pxtnOK) {
      dump_error(ppErrPxtoneInitFailure);
      return false;
    }

    // set quality
    {
      unsigned long channels, rate;
      opt.get("channels", channels);
      opt.get("rate", rate);

      if (!pxtn.set_destination_quality(channels, rate)) {
        dump_error(ppErrPxtoneSetQuality);
        return false;
      }
    }

    // load file
    if (!load_ptcop(pxtn, opt.dumpInputfile().c_str(), pxtn_err)) {
      dump_error(ppErrCantLoadPtcopFile);
      return false;
    }

    // show pxtone file info
    std::cout << "------------- music info -------------" << std::endl;
    std::cout << "Title: "
              << util::MS932toUTF8(pxtn.text->get_name_buf(&buf_size))
              << std::endl;
    std::cout << "  Comment: "
              << util::MS932toUTF8(pxtn.text->get_comment_buf(&buf_size))
              << std::endl;

    std::cout << "  beat: " << pxtn.master->get_beat_num() << std::endl;
    std::cout << "  beat tempo: " << pxtn.master->get_beat_tempo() << std::endl;
    std::cout << "  beat clock: " << pxtn.master->get_beat_clock() << std::endl;
    std::cout << "  measure: " << pxtn.master->get_meas_num() << std::endl;

    {
      // set loop and volume
      bool loop = false;
      unsigned long volume;

      opt.get("loop", loop);
      opt.get("volume", volume);

      int32_t smp_total = pxtn.moo_get_total_sample();

      pxtnVOMITPREPARATION prep = {0};
      prep.flags |= (loop ? pxtnVOMITPREPFLAG_loop : 0);
      prep.start_pos_float = 0;
      prep.master_volume = (volume / 100.0f);

      if (!pxtn.moo_preparation(&prep)) {
        dump_error(ppErrPxtoneSetLoop);
        return false;
      }
    }

    // play
    {
      bool dummy;
      opt.get("dummy", dummy);
      if (dummy) {
        return dummy_play(opt, pxtn);
      } else {
        return alsa_play(opt, pxtn);
      }
    }
  }

  return true;
}

void show_player_info(option::ppOption &opt)
{
  // get config
  unsigned long channels, rate, buffersize;
  opt.get("channels", channels);
  opt.get("rate", rate);
  opt.get("buffer-size", buffersize);

  // clang-format off
  std::cout << "----------- player config ------------"                  << std::endl;
  std::cout << "  Quality: " << rate << "Hz / "
		        << (pxtnBITPERSAMPLE == 8 ? " 8"     : "16"    )             << "bit / "
            << (channels == 1         ? "  mono" : "stereo")             << std::endl;
  std::cout << "  Buffer size: " << std::right << std::setw(15)
            << (buffersize * channels * pxtnBITPERSAMPLE / 8) << " byte" << std::endl;
  std::cout << "--------------------------------------"                  << std::endl;
  // clang-format on
}

void show_play_time(double span)
{
  static size_t count = 0;
  std::cout << "Time: " << util::printAsTime(count * span) << "\r";
  count++;
}

bool alsa_play(option::ppOption &opt, pxtnService &pxtn)
{
  int err;
  snd_pcm_t *pv_h;
  snd_pcm_hw_params_t *hw_params;

  // get config
  snd_pcm_format_t hw_format;
  unsigned long channels, rate, buffersize, fadein, fadeout;
  std::string device;
  opt.get("channels", channels);
  opt.get("rate", rate);
  opt.get("buffer-size", buffersize);
  opt.get("device", device);
  opt.get("fadein", fadein);
  opt.get("fadeout", fadeout);

  if (pxtnBITPERSAMPLE == 8) {
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
  unsigned long framesize = pxtnBITPERSAMPLE / 8 * channels;
  std::vector<std::uint8_t> buf((size_t)framesize * buffersize, 0);

  show_player_info(opt);

  // play
  if (fadein > 0 && !pxtn.moo_set_fade(1, fadein / 1000.0f)) {
    dump_error(ppErrPxtoneStartFailure);
    return false;
  }

  double vomit_span = (double)buffersize / rate;
  int32_t res_size = buf.size();
  while (pxtn.Moo(buf.data(), res_size) && res_size > 0) {
    show_play_time(vomit_span);

    auto frame = snd_pcm_writei(pv_h, buf.data(), res_size);

    if (frame < 0) {
      if (snd_pcm_recover(pv_h, frame, 0) < 0) {
        dump_error(ppErrAlsaWriteInterface);
        dump_alsa_error(snd_strerror(err));
        snd_pcm_close(pv_h);
        return false;
      }
    }
    res_size = buf.size();
  }
  std::cout << std::endl;

  snd_pcm_close(pv_h);
  return true;
}
bool dummy_play(option::ppOption &opt, pxtnService &pxtn)
{
  // get config
  unsigned long channels, rate, buffersize, fadein, fadeout;
  opt.get("channels", channels);
  opt.get("rate", rate);
  opt.get("buffer-size", buffersize);
  opt.get("fadein", fadein);
  opt.get("fadeout", fadeout);

  // create buffer
  unsigned long framesize = pxtnBITPERSAMPLE / 8 * channels;
  std::vector<std::uint8_t> buf((size_t)framesize * buffersize, 0);

  show_player_info(opt);

  // play
  if (fadein > 0 && !pxtn.moo_set_fade(1, fadein / 1000.0f)) {
    dump_error(ppErrPxtoneStartFailure);
    return false;
  }

  double vomit_span = (double)buffersize / rate;
  std::cout << buf.size() << std::endl;
  int32_t res_size = buf.size();

  while (pxtn.Moo(buf.data(), res_size) && res_size > 0) {
    show_play_time(vomit_span);
    res_size = buf.size();
  }
  std::cout << std::endl;

  return true;
}
}
