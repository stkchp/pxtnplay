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
constexpr auto& ppErrCantReadFileIntoMemory =
// ------------------------------------------------------------------------
R"(Error: Can't read file to memory.)" "\n"
R"(Note:  pxtnplay limit filesize upto 10Mb.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrPxtoneInitFailure =
R"(Error: (Pxtone) Can't Initialize Pxtone Library.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrPxtoneReadAsPxtone =
R"(Error: (Pxtone) Read as ptcol file failure.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrPxtoneStartFailure =
R"(Error: (Pxtone) Cannot start creating buffer data.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrAlsaOpenDevice =
R"(Error: (ALSA) Cannot open audio device.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrAlsaAllocateParameter =
R"(Error: (ALSA) Cannot initialize hardware parameter structure.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrAlsaSetAccessType =
R"(Error: (ALSA) Cannot set access type.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrAlsaSetSampleFormat =
R"(Error: (ALSA) Cannot set sample format.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrAlsaSetRateNear =
R"(Error: (ALSA) Cannot set sample rate.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrAlsaSetChannels =
R"(Error: (ALSA) Cannot set sample rate.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrAlsaSetParameters =
R"(Error: (ALSA) Cannot set sample rate.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrAlsaPrepareInterface =
R"(Error: (ALSA) Cannot prepare audio interface for use.)";
// ------------------------------------------------------------------------
constexpr auto& ppErrAlsaWriteInterface =
R"(Error: (ALSA) Write to audio interface failed.)";
// ------------------------------------------------------------------------

// clang-format on
}
}
