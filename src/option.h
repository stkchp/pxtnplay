#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include <getopt.h>
#include <unistd.h>

namespace pxtnplay
{
namespace option
{
struct ppElement {
  char sopt;
  std::string lopt;
  std::string limit;
  std::string type;
  bool config;
};
ppElement make_element(char, std::string &, std::string &, std::string &, bool);

using pair_index_t = std::pair<std::string, std::size_t>;
using pair_data_t = std::pair<std::uint8_t, std::string>;
using map_data_t = std::multimap<size_t, pair_data_t>;

class ppOption
{
  std::string _help;

  std::string _inputfile;

  std::vector<ppElement> _element;
  std::map<std::string, size_t> _m_lopt;
  std::map<std::string, size_t> _m_sopt;
  map_data_t _m_data;

  void _appendHelp(std::string &str);

  bool _set(const char *lopt, const char *data, std::uint8_t priority);

  std::string _makeShortOptions();
  void _makeLongOptions(std::vector<::option> &values);

public:
  void parseHelp(const char *str);            // priority 0 (default value)
  bool parseCommand(int argc, char *argv[]);  // priority 255
  bool parseConfig(const char *path, std::uint8_t priority);  // not

  const char *dumpHelp();
  const std::string &dumpInputfile();
  void dumpOptions();

  bool get(const char *name, bool &value);
  bool get(const char *name, unsigned long &value);
  bool get(const char *name, std::string &value);

  bool gets(const char *name, std::vector<std::string> &values);
};
}
}
