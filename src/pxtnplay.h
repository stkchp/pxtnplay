#pragma once

#include <pxTypedef.h>

#define PXTNPLAY_VERSION "0.0.1"

namespace pxtnplay
{

bool run_pxtnplay(int argc, char *argv[]);
}

// add function to core/pxtnWoisePTV.cpp to show
// library version.
s32 get_pxtnVersion();
