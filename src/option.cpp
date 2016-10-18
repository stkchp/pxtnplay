#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <regex>
#include <sstream>
#include <string>

#include <getopt.h>

#include "option.h"

#define GETOPT_LONG_INDEX (0x100)

//
// Utility functions
//
namespace
{
bool parseBool(const std::string &in)
{
  if (in == "true" || in == "1") {
    return true;
  }
  return false;
}
unsigned long parseInteger(const std::string &in)
{
  static const char uint_txt[] = R"([1-9][0-9]*)";
  static std::regex uint_reg(uint_txt);
  std::smatch match;

  if (std::regex_match(in, match, uint_reg)) {
    unsigned long ret;
    std::stringstream(match[0].str()) >> ret;
    return ret;
  } else {
    return 0;
  }
}

bool validateInteger(const std::string &in, const std::string &limit)
{
  auto t = parseInteger(in);
  decltype(t) b_s = 0;
  decltype(t) c_s = 0;
  std::string s = "";
  bool range = false;

  for (auto it = limit.begin(); it != limit.end(); ++it) {
    if (std::isdigit(*it)) {
      s += *it;
    } else {
      b_s = c_s;
      c_s = parseInteger(s);
      s = "";

      if (*it == ',') {
        if (range) {
          if (b_s <= t && t <= c_s) return true;
        } else {
          if (t == c_s) return true;
        }
        range = false;
      } else if (*it == '-') {
        range = true;
      } else {
        // not support format
        return false;
      }
    }
  }

  b_s = c_s;
  c_s = parseInteger(s);
  if (range) {
    if (b_s <= t && t <= c_s) return true;
  } else {
    if (t == c_s) return true;
  }
  return false;
}
}

//
// pxtnplay::option
//
namespace pxtnplay
{
namespace option
{
ppElement make_element(char _sopt, std::string &_lopt, std::string &_limit,
                       std::string &_type, bool _config)
{
  ppElement element;
  element.sopt = _sopt;
  element.lopt = _lopt;
  element.limit = _limit;
  element.type = _type;
  element.config = _config;
  return element;
}

void ppOption::_appendHelp(std::string &str)
{
  _help += str;
  _help += "\n";
}

bool ppOption::_set(const char *lopt, const char *data, std::uint8_t priority)
{
  std::string _data(data);

  auto it = _m_lopt.find(lopt);
  if (it == _m_lopt.end()) return false;
  auto index = it->second;
  auto &v = _element.at(index);
  if (v.type == "int") {
    // if limit exist, check value.
    if (v.limit.size() > 0 && !validateInteger(_data, v.limit)) {
      // print error and exit
      std::cerr << "Error: '" << v.lopt << "' option must be " << v.limit << "."
                << std::endl;

      return false;
    }
  }
  _m_data.insert(
      std::pair<size_t, pair_data_t>(index, pair_data_t(priority, data)));

  return true;
}

bool ppOption::get(const char *name, bool &values)
{
  auto it = _m_lopt.find(name);
  if (it == _m_lopt.end()) return false;
  auto index = it->second;
  if (_element.at(index).type != "bool") return false;

  std::string result;
  if (!get(name, result)) return false;

  values = parseBool(result);
  return true;
}

bool ppOption::get(const char *name, unsigned long &values)
{
  auto it = _m_lopt.find(name);
  if (it == _m_lopt.end()) return false;
  auto index = it->second;
  if (_element.at(index).type != "int") return false;

  std::string result;
  if (!get(name, result)) return false;

  values = parseInteger(result);
  return true;
}

bool ppOption::get(const char *name, std::string &values)
{
  std::vector<std::string> tmp;
  if (!gets(name, tmp)) return false;
  if (tmp.size() == 0) return false;
  values = tmp.at(0);
  return true;
}

bool ppOption::gets(const char *name, std::vector<std::string> &values)
{
  auto it = _m_lopt.find(name);
  if (it == _m_lopt.end()) return false;
  auto index = it->second;

  auto range = _m_data.equal_range(index);
  std::vector<pair_data_t> tmp;

  // get pair matching key
  std::for_each(range.first, range.second,
                [&tmp](const std::pair<size_t, pair_data_t> &x) {
                  tmp.push_back(x.second);
                });

  std::sort(
      tmp.begin(), tmp.end(), [](const pair_data_t &a, const pair_data_t &b) {
        return b.first == a.first ? b.second > a.second : b.first < a.first;
      });

  for (auto &v : tmp) {
    values.push_back(v.second);
  }
  return true;
}

std::string ppOption::_makeShortOptions()
{
  std::string s;
  for (auto v : _element) {
    if (v.sopt == 0) continue;
    s += v.sopt;
    if (v.type == "bool") continue;
    s += ":";
  }
  return s;
}

void ppOption::_makeLongOptions(std::vector<::option> &values)
{
  for (size_t i = 0; i < _element.size(); i++) {
    ::option opt = {                                     // please see getopt.h
                    _element[i].lopt.c_str(),            //
                    _element[i].type == "bool" ? 0 : 1,  //
                    nullptr,                             //
                    i + GETOPT_LONG_INDEX};
    values.push_back(opt);
  }
  ::option opt = {nullptr, 0, nullptr, 0};
  values.push_back(opt);
}

void ppOption::parseHelp(const char *str)
{
  // regex format
  // clang-format off
  static const char r_str[] =
    R"((^[ \t]*(-([[:alnum:]])[ \t]+|())--([a-zA-Z0-9\-_]+)[^\(#]*(\(([^\)]+)\)|())[^\(#]*))"
    R"(#[ \t]*([\w]+)[ \t]*<([^>]*)>[ \t]*<([0-9]+)>.*$)";
  // clang-format on
  std::regex reg(r_str);
  std::smatch match;

  std::istringstream iss(str);
  for (std::string line; std::getline(iss, line);) {
    if (std::regex_match(line, match, reg)) {
      // save to help
      _help += match[1].str();
      _help += "\n";

      size_t index = _element.size();

      char sopt = 0;
      auto lopt = match[5].str();
      auto limit = match[7].str();
      auto opttype = match[9].str();
      if (match[3].str().size() > 0) sopt = match[3].str().at(0);

      if (opttype != "bool" && opttype != "int") {
        opttype = "string";
      }
      _element.push_back(
          // add option element
          make_element(sopt,                       // short option
                       lopt,                       // long option
                       limit,                      // value limitation
                       opttype,                    // opttype(bool,int,string)
                       parseBool(match[11].str())  // read from config?
                       ));
      // update index map
      _m_lopt.insert(pair_index_t(match[5].str(), index));
      if (match[3].str().size() > 0)
        _m_sopt.insert(pair_index_t(match[3].str(), index));

      // set default value
      _set(_element.at(index).lopt.c_str(), match[10].str().c_str(), 0);
    } else {
      // save to help
      _help += line;
      _help += "\n";
    }
  }
}

bool ppOption::parseCommand(int argc, char *argv[])
{
  // parsing argument
  std::string short_options = _makeShortOptions();
  std::vector<::option> long_options;
  _makeLongOptions(long_options);

  int c, option_index;

  while ((c = getopt_long(argc, argv, short_options.c_str(),
                          long_options.data(), &option_index)) != -1) {
    size_t index = std::numeric_limits<size_t>::max();

    if (c < GETOPT_LONG_INDEX) {
      std::string _c("");
      _c += (char)(c & 0xFF);
      std::string sopt(_c);
      auto it = _m_sopt.find(sopt);
      if (it == _m_sopt.end()) continue;
      index = it->second;
    } else {
      index = c - GETOPT_LONG_INDEX;
    }
    if (index >= _element.size()) return false;
    if (_element.at(index).type == "bool") {
      if (!_set(_element.at(index).lopt.c_str(), "true", 255)) return false;
    } else {
      if (optarg != nullptr) {
        if (!_set(_element.at(index).lopt.c_str(), optarg, 255)) return false;
      }
    }
  }
  if (optind < argc) {
    _inputfile = argv[optind];
  }

  // for (auto v : _m_data) {
  //   std::cout << "---- data ----" << std::endl;
  //   std::cout << _element.at(v.first).lopt << std::endl              //
  //             << "  priority: " << (int)v.second.first << std::endl  //
  //             << "  data    : " << v.second.second << std::endl;
  // }

  return true;
}

bool ppOption::parseConfig(const char *path, std::uint8_t priority)
{
  return false;
}
const char *ppOption::dumpHelp() { return _help.c_str(); }
const std::string &ppOption::dumpInputfile() { return _inputfile; }
void ppOption::dumpOptions()
{
  for (auto v : _element) {
    std::cout << v.lopt << "(" << ((v.sopt) > 0 ? v.sopt : '\0') << "): ";
    if (v.type == "bool") {
      bool res = false;
      get(v.lopt.c_str(), res);
      std::cout << res << std::endl;
    } else if (v.type == "int") {
      unsigned long res = 0;
      get(v.lopt.c_str(), res);
      std::cout << res << std::endl;
    } else {
      std::string res("");
      get(v.lopt.c_str(), res);
      std::cout << res << std::endl;
    }
  }
}
}
}
