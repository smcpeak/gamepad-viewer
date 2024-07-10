// gpv-config.cc
// Code for `gpv-config` module.

// See license.txt for copyright and terms of use.

#include "gpv-config.h"                // this module

#include "json.hpp"                    // json::...

#include <windows.h>                   // COLORREF

#include <cerrno>                      // errno
#include <cstring>                     // std::strerror
#include <fstream>                     // std::ofstream
#include <sstream>                     // std::ostringstream

using json::JSON;


GPVConfig::GPVConfig()
  : m_linesColorref(RGB(118, 235, 220))          // Pastel cyan.
{}


static JSON COLORREF_to_JSON(COLORREF cr)
{
  int r = GetRValue(cr);
  int g = GetGValue(cr);
  int b = GetBValue(cr);

  return json::Array(r,g,b);
}


static COLORREF COLORREF_from_JSON(JSON arr)
{
  if (arr.length() >= 3) {
    int r = arr[0].ToInt();
    int g = arr[1].ToInt();
    int b = arr[2].ToInt();

    return RGB(r,g,b);
  }
  else {
    // We don't have proper exception infrastructure here.
    return RGB(0,0,0);
  }
}


std::string GPVConfig::loadFromFile(std::string const &fname)
{
  std::ifstream in(fname, std::ios::binary);
  if (!in) {
    return std::strerror(errno);
  }

  std::ostringstream oss;
  oss << in.rdbuf();

  // This merely reports errors to stderr...
  JSON obj = JSON::Load(oss.str());

  if (obj.hasKey("linesColorRGB")) {
    m_linesColorref = COLORREF_from_JSON(obj["linesColorRGB"]);
  }

  return "";
}


std::string GPVConfig::saveToFile(std::string const &fname) const
{
  JSON obj = json::Object();

  obj["linesColorRGB"] = COLORREF_to_JSON(m_linesColorref);

  std::string serialized = obj.dump();

  std::ofstream out(fname, std::ios::binary);
  if (out) {
    out << serialized << "\n";
    return "";
  }
  else {
    return std::strerror(errno);
  }
}


// EOF
