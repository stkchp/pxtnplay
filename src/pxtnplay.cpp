#include <cstdlib>
#include <iostream>

#include <getopt.h>
#include <unistd.h>

#include <pxtnService.h>

#include "pxtnplay.h"

namespace
{
using std::cout;
using std::cerr;
using std::endl;

void dump_help(char *command)
{
	cout << "Usage: " << command;
	cout << " [options] FILE\n"
		"Play Pxtone Colage music files (.ptcop /.pptune) in command line.\n"
		"\n"
		"General Options\n"
		"  -h --help       show this help and exit\n"
		"  -v --version    show version and exit\n"
		"  -q --quiet      no output\n"
		"\n"
		"ALSA Options\n"
		"  not implemented yet\n"
		"\n"
		"Pxtone Options\n"
		"  -l --loop       enable loop\n"
		"  -c --channel    pxtone channel\n"
	     << endl;
};
void dump_version()
{
	cout << "pxtnplay: " PXTNPLAY_VERSION << endl;
	cout << "Pxtone Collage Library: " << get_pxtnVersion() << endl;
	cout << "--------------------------------------------------\n"
		"Copyright (c) 2016 STUDIO PIXEL\n"
		"Copyright (c) 2016 stkchp\n"
		"This software is released under the MIT License.\n"
		"  http://opensource.org/licenses/mit-license.php\n"
		"--------------------------------------------------\n"
	     << endl;
}
void dump_channel_over() { cerr << "Error: channel number must be 1 or 2." << endl; }
void dump_see_help(char *arg) { cerr << "Error: unknown option '" << arg << "'." << endl; }
void dump_needfile()
{
	// need input file specified.
	cerr << "Error: no input files." << endl;
}
}

namespace pxtnplay
{
namespace optionType
{
enum : int {
	LOOP = 1,

};
}
class ppOption
{
public:
	ppOption();
	bool help;
	bool version;
	bool quiet;
	bool loop;
	uint32_t channel;
	char *filename;
};
ppOption::ppOption()
{
	// option default value
	help = false;
	version = false;
	quiet = false;
	loop = false;
	channel = 2;
	filename = nullptr;
}
bool run_pxtnplay(int argc, char *argv[])
{
	// parse option
	ppOption opt;

	int c, option_index;
	static const char short_options[] = "hvqlc:";
	static const struct option long_options[] = {
	    {"help", 0, 0, 'h'},    //
	    {"version", 0, 0, 'v'}, //
	    {"quiet", 0, 0, 'q'},   //
	    {"loop", 0, 0, 'l'},    //
	    {"channel", 1, 0, 'c'}, //
	    {nullptr, 0, 0, 0}      //
	};

	while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch (c) {
		case 'h':
			opt.help = true;
			break;
		case 'v':
			opt.version = true;
			break;
		case 'q':
			opt.quiet = true;
			break;
		case 'l':
			opt.loop = true;
			break;
		case 'c':
			if (optarg != nullptr) {
				char *end;
				opt.channel = std::strtoul(optarg, &end, 10);
			}
			break;
		default:
			dump_see_help(optarg);
			break;
		}
	}
	if (optind < argc) {
		opt.filename = argv[optind];
	}

	// option check
	if (opt.help) {
		dump_help(argv[0]);
		return true;
	}
	if (opt.version) {
		dump_version();
		return true;
	}
	if (opt.channel > 2) {
		dump_channel_over();
		return false;
	}
	if (opt.filename == nullptr) {
		dump_needfile();
		return false;
	}

	return true;
}
}
