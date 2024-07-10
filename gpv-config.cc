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
  : m_linesColorref(RGB(118, 235, 220)),         // Pastel cyan.
    m_highlightColorref(RGB(53, 53, 242)),       // Dark blue, almost purple.
    m_showText(false),
    m_topmostWindow(false),
    m_windowLeft(50),
    m_windowTop(300),
    m_windowWidth(400),
    m_windowHeight(400),
    m_pollingIntervalMS(16),                     // ~60 FPS.
    m_controllerID(0)                            // First controller.
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

  // Load `field` from `key`.  `expr` is an expression that refers to
  // `data`, the JSON object at `key`.
  #define LOAD_FIELD(key, field, expr) \
    if (obj.hasKey(key)) {             \
      JSON const &data = obj[key];     \
      field = (expr);                  \
    }

  // For when the key and field names are related in the usual way.
  #define LOAD_KEY_FIELD(name, expr) \
    LOAD_FIELD(#name, m_##name, expr)

  LOAD_FIELD("linesColorRGB", m_linesColorref, COLORREF_from_JSON(data));
  LOAD_FIELD("highlightColorRGB", m_highlightColorref, COLORREF_from_JSON(data));
  LOAD_KEY_FIELD(showText, data.ToBool());
  LOAD_KEY_FIELD(topmostWindow, data.ToBool());
  LOAD_KEY_FIELD(windowLeft, data.ToInt());
  LOAD_KEY_FIELD(windowTop, data.ToInt());
  LOAD_KEY_FIELD(windowWidth, data.ToInt());
  LOAD_KEY_FIELD(windowHeight, data.ToInt());
  LOAD_KEY_FIELD(pollingIntervalMS, data.ToInt());
  LOAD_KEY_FIELD(controllerID, data.ToInt());

  #undef LOAD_FIELD
  #undef LOAD_KEY_FIELD

  return "";
}


std::string GPVConfig::saveToFile(std::string const &fname) const
{
  JSON obj = json::Object();

  obj["linesColorRGB"] = COLORREF_to_JSON(m_linesColorref);
  obj["highlightColorRGB"] = COLORREF_to_JSON(m_highlightColorref);

  // Save a field where converting to JSON only requires invoking one of
  // the `JSON` constructors.
  #define SAVE_KEY_FIELD_CTOR(name) \
    obj[#name] = JSON(m_##name) /* user ; */

  SAVE_KEY_FIELD_CTOR(showText);
  SAVE_KEY_FIELD_CTOR(topmostWindow);
  SAVE_KEY_FIELD_CTOR(windowLeft);
  SAVE_KEY_FIELD_CTOR(windowTop);
  SAVE_KEY_FIELD_CTOR(windowWidth);
  SAVE_KEY_FIELD_CTOR(windowHeight);
  SAVE_KEY_FIELD_CTOR(pollingIntervalMS);
  SAVE_KEY_FIELD_CTOR(controllerID);

  #undef SAVE_KEY_FIELD_CTOR

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
