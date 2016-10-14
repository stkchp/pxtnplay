#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "pxtnplay.h"

namespace
{
using std::cerr;

void signal_handler(int signal)
{
	if (signal == SIGSEGV) {
		cerr << "Error: Received SIGSEGV signal...\n"
			"       Please report bug to https://github.com/stkchp/pxtnplay\n"
			"       with your environment infomation.\n"
			"       Maybe bug located in pxtnplay, not in pxtone source.\n"
			"       PLEASE DO NOT report to Amaya Pixel.\n";
		exit(EXIT_FAILURE);
	}
};
}

int main(int argc, char *argv[])
{

// segv handler

#ifdef HAVE_SIGACTION
	struct sigaction act;
	act.sa_handler = signal_handler;
	act.sa_flags   = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGSEGV, &act, std::nullptr);
#else
	std::signal(SIGSEGV, signal_handler);
#endif

	auto r = pxtnplay::run_pxtnplay(argc, argv);

	return r ? EXIT_SUCCESS : EXIT_FAILURE;
}
