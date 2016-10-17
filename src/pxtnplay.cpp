#include <cstdlib>
#include <fstream>
#include <iostream>

#include <pxtnService.h>

#include "config.h"
#include "option.h"
#include "pxtnplay.h"

namespace
{
using std::cout;
using std::cerr;
using std::endl;

// clang-format off
const char help_string[] =
/*---------------------------------------------------------------------- # type <default> <from config> */
"Usage: pxtnplay [options] FILE\n"
"Play Pxtone Colage music files (.ptcop /.pptune) in command line.\n"
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
};
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
void dump_channel_over()
{
  cerr << "Error: channel number must be 1 or 2." << endl;
}
void dump_see_help(const char *arg)
{
  cerr << "Error: unknown option '" << arg << "'." << endl;
}
void dump_needfile()
{
  // need input file specified.
  cerr << "Error: no input files." << endl;
}
void dump_cannot_open()
{
  // need input file specified.
  cerr << "Error: Can't open file." << endl;
}
}

namespace pxtnplay
{
bool run_pxtnplay(int argc, char *argv[])
{
  // parse option
  option::ppOption opt;

  opt.parseHelp(help_string);

  // command line parse error.
  if (!opt.parseCommand(argc, argv)) return false;

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
    dump_needfile();
    return false;
  }

  // config file parsing error handling
  opt.dumpOptions();

  // check file exists
  {
    std::ifstream f(opt.dumpInputfile());
    if (!f.good()) {
      dump_cannot_open();
      return false;
    }
  }

  return true;
}
}
