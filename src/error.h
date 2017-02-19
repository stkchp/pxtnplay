#pragma once

namespace pxtnplay
{
namespace error
{
// clang-format off
// ------------------------------------------------------------------------
constexpr auto& ppErrNoInputFile = 
R"(Error: No input files.)" "\n"
R"(       Please specify Pxtone Music file.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrCantOpenFile =
R"(Error: Can't open file(fopen error).)" "\n"
R"(       Please specify Pxtone Music file.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrCantLoadPtcopFile =
R"(Error: Can't load file as pxtone file.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrPxtoneInitFailure =
R"(Error: (Pxtone) Can't Initialize Pxtone Library.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrPxtoneReadAsPxtone =
R"(Error: (Pxtone) Read as ptcop file failure.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrPxtoneSetLoop =
R"(Error: (Pxtone) Cannot setup loop and volume.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrPxtoneSetQuality =
R"(Error: (Pxtone) Cannot set quality(channels, rate).)";
// ------------------------------------------------------------------------
constexpr auto& ppErrPxtoneStartFailure =
R"(Error: (Pxtone) Cannot start creating buffer data.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrLibaoOpenLive =
R"(Error: (libao) Cannot open audio device.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrLibaoPlay =
R"(Error: (libao) Something happend in playing sound.)";
// ------------------------------------------------------------------------

// clang-format on
}
}
