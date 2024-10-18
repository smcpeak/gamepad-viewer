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


// Load `field` from `key`.  `expr` is an expression that refers to
// `data`, the JSON object at `key`.
#define LOAD_FIELD(key, field, expr) \
  if (obj.hasKey(key)) {             \
    JSON const &data = obj.at(key);  \
    field = (expr);                  \
  }

// For when the key and field names are related in the usual way.
#define LOAD_KEY_FIELD(name, expr) \
  LOAD_FIELD(#name, m_##name, expr)


// For when the data type is `int`.
#define LOAD_KEY_INT_FIELD(name) \
  LOAD_FIELD(#name, m_##name, data.ToInt())

// For when the data type is `float`.
#define LOAD_KEY_FLOAT_FIELD(name) \
  LOAD_FIELD(#name, m_##name, data.ToFloat())


// Save a field where converting to JSON only requires invoking one of
// the `JSON` constructors.
#define SAVE_KEY_FIELD_CTOR(name) \
  obj[#name] = JSON(m_##name) /* user ; */


// ----------------------- AnalogThresholdConfig -----------------------
AnalogThresholdConfig::AnalogThresholdConfig()
    // These defaults are tuned for Elden Ring.
    //
    // TODO: Shouldn't the dead zone be 127?  At 128 it fires, right?
  : m_triggerDeadZone(128),
    m_rightStickDeadZone(6600),
    m_leftStickWalkThreshold(16000),
    m_leftStickRunThreshold(25500),
    m_leftStickSprintThreshold(30000)
{}


void AnalogThresholdConfig::loadFromJSON(JSON const &obj)
{
  LOAD_KEY_FIELD(triggerDeadZone, data.ToInt());
  LOAD_KEY_FIELD(rightStickDeadZone, data.ToInt());
  LOAD_KEY_FIELD(leftStickWalkThreshold, data.ToInt());
  LOAD_KEY_FIELD(leftStickRunThreshold, data.ToInt());
  LOAD_KEY_FIELD(leftStickSprintThreshold, data.ToInt());
}


JSON AnalogThresholdConfig::saveToJSON() const
{
  JSON obj = json::Object();

  SAVE_KEY_FIELD_CTOR(triggerDeadZone);
  SAVE_KEY_FIELD_CTOR(rightStickDeadZone);
  SAVE_KEY_FIELD_CTOR(leftStickWalkThreshold);
  SAVE_KEY_FIELD_CTOR(leftStickRunThreshold);
  SAVE_KEY_FIELD_CTOR(leftStickSprintThreshold);

  return obj;
}


// ------------------------- ParryTimerConfig --------------------------
ParryTimerConfig::ParryTimerConfig()
  // Defaults in class body.
{}


bool ParryTimerConfig::isActive(int elapsedMS) const
{
  return m_activeStartMS <= elapsedMS &&
                            elapsedMS <= m_activeEndMS;
}


void ParryTimerConfig::loadFromJSON(json::JSON const &obj)
{
  LOAD_KEY_INT_FIELD(durationMS);
  LOAD_KEY_INT_FIELD(numSegments);
  LOAD_KEY_INT_FIELD(activeStartMS);
  LOAD_KEY_INT_FIELD(activeEndMS);
}


json::JSON ParryTimerConfig::saveToJSON() const
{
  JSON obj = json::Object();

  SAVE_KEY_FIELD_CTOR(durationMS);
  SAVE_KEY_FIELD_CTOR(numSegments);
  SAVE_KEY_FIELD_CTOR(activeStartMS);
  SAVE_KEY_FIELD_CTOR(activeEndMS);

  return obj;
}


// --------------------------- LayoutParams ----------------------------
LayoutParams::LayoutParams()
  // All of the default values are in the class body.
{}


void LayoutParams::loadFromJSON(JSON const &obj)
{
  LOAD_KEY_FLOAT_FIELD(textFontSizeDIPs);
  LOAD_KEY_FLOAT_FIELD(faceButtonsY);
  LOAD_KEY_FLOAT_FIELD(faceButtonsR);
  LOAD_KEY_FLOAT_FIELD(roundButtonR);
  LOAD_KEY_FLOAT_FIELD(dpadButtonR);
  LOAD_KEY_FLOAT_FIELD(shoulderButtonsX);
  LOAD_KEY_FLOAT_FIELD(shoulderButtonsR);
  LOAD_KEY_FLOAT_FIELD(bumperVR);
  LOAD_KEY_FLOAT_FIELD(triggerVR);
  LOAD_KEY_FLOAT_FIELD(parryTimerX);
  LOAD_KEY_FLOAT_FIELD(parryTimerY);
  LOAD_KEY_FLOAT_FIELD(parryTimerHR);
  LOAD_KEY_FLOAT_FIELD(parryTimerVR);
  LOAD_KEY_FLOAT_FIELD(parryTimerHashHeight);
  LOAD_KEY_FLOAT_FIELD(stickR);
  LOAD_KEY_FLOAT_FIELD(stickOutlineR);
  LOAD_KEY_FLOAT_FIELD(stickMaxDeflectR);
  LOAD_KEY_FLOAT_FIELD(stickThumbR);
  LOAD_KEY_FLOAT_FIELD(chevronSeparation);
  LOAD_KEY_FLOAT_FIELD(chevronHR);
  LOAD_KEY_FLOAT_FIELD(chevronVR);
  LOAD_KEY_FLOAT_FIELD(selStartX);
  LOAD_KEY_FLOAT_FIELD(selStartHR);
  LOAD_KEY_FLOAT_FIELD(selStartVR);
  LOAD_KEY_FLOAT_FIELD(centralCircleY);
  LOAD_KEY_FLOAT_FIELD(centralCircleR);
  LOAD_KEY_FLOAT_FIELD(circleMargin);
  LOAD_KEY_FLOAT_FIELD(lineWidthPixels);
}


JSON LayoutParams::saveToJSON() const
{
  JSON obj = json::Object();

  SAVE_KEY_FIELD_CTOR(textFontSizeDIPs);
  SAVE_KEY_FIELD_CTOR(faceButtonsY);
  SAVE_KEY_FIELD_CTOR(faceButtonsR);
  SAVE_KEY_FIELD_CTOR(roundButtonR);
  SAVE_KEY_FIELD_CTOR(dpadButtonR);
  SAVE_KEY_FIELD_CTOR(shoulderButtonsX);
  SAVE_KEY_FIELD_CTOR(shoulderButtonsR);
  SAVE_KEY_FIELD_CTOR(bumperVR);
  SAVE_KEY_FIELD_CTOR(triggerVR);
  SAVE_KEY_FIELD_CTOR(parryTimerX);
  SAVE_KEY_FIELD_CTOR(parryTimerY);
  SAVE_KEY_FIELD_CTOR(parryTimerHR);
  SAVE_KEY_FIELD_CTOR(parryTimerVR);
  SAVE_KEY_FIELD_CTOR(parryTimerHashHeight);
  SAVE_KEY_FIELD_CTOR(stickR);
  SAVE_KEY_FIELD_CTOR(stickOutlineR);
  SAVE_KEY_FIELD_CTOR(stickMaxDeflectR);
  SAVE_KEY_FIELD_CTOR(stickThumbR);
  SAVE_KEY_FIELD_CTOR(chevronSeparation);
  SAVE_KEY_FIELD_CTOR(chevronHR);
  SAVE_KEY_FIELD_CTOR(chevronVR);
  SAVE_KEY_FIELD_CTOR(selStartX);
  SAVE_KEY_FIELD_CTOR(selStartHR);
  SAVE_KEY_FIELD_CTOR(selStartVR);
  SAVE_KEY_FIELD_CTOR(centralCircleY);
  SAVE_KEY_FIELD_CTOR(centralCircleR);
  SAVE_KEY_FIELD_CTOR(circleMargin);
  SAVE_KEY_FIELD_CTOR(lineWidthPixels);

  return obj;
}


// ----------------------------- GPVConfig -----------------------------
GPVConfig::GPVConfig()
  : m_linesColorref(RGB(118, 235, 220)),         // Pastel cyan.
    m_highlightColorref(RGB(53, 53, 242)),       // Dark blue, almost purple.
    m_parryActiveColorref(RGB(255, 0, 0)),
    m_parryInactiveColorref(RGB(128, 128, 128)),
    m_showText(false),
    m_topmostWindow(false),
    m_windowLeft(50),
    m_windowTop(300),
    m_windowWidth(400),
    m_windowHeight(400),
    m_pollingIntervalMS(16),                     // ~60 FPS.
    m_controllerID(0),                           // First controller.
    m_analogThresholds(),
    m_layoutParams()
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


void GPVConfig::loadFromJSON(JSON const &obj)
{
  LOAD_FIELD("linesColorRGB", m_linesColorref, COLORREF_from_JSON(data));
  LOAD_FIELD("highlightColorRGB", m_highlightColorref, COLORREF_from_JSON(data));
  LOAD_FIELD("parryActiveColorRGB", m_parryActiveColorref, COLORREF_from_JSON(data));
  LOAD_FIELD("parryInactiveColorRGB", m_parryInactiveColorref, COLORREF_from_JSON(data));

  LOAD_KEY_FIELD(showText, data.ToBool());
  LOAD_KEY_FIELD(topmostWindow, data.ToBool());
  LOAD_KEY_FIELD(windowLeft, data.ToInt());
  LOAD_KEY_FIELD(windowTop, data.ToInt());
  LOAD_KEY_FIELD(windowWidth, data.ToInt());
  LOAD_KEY_FIELD(windowHeight, data.ToInt());
  LOAD_KEY_FIELD(pollingIntervalMS, data.ToInt());
  LOAD_KEY_FIELD(controllerID, data.ToInt());

  if (obj.hasKey("analogThresholds")) {
    m_analogThresholds.loadFromJSON(obj.at("analogThresholds"));
  }

  if (obj.hasKey("parryTimer")) {
    m_parryTimer.loadFromJSON(obj.at("parryTimer"));
  }

  if (obj.hasKey("layoutParams")) {
    m_layoutParams.loadFromJSON(obj.at("layoutParams"));
  }
}


JSON GPVConfig::saveToJSON() const
{
  JSON obj = json::Object();

  obj["linesColorRGB"] = COLORREF_to_JSON(m_linesColorref);
  obj["highlightColorRGB"] = COLORREF_to_JSON(m_highlightColorref);
  obj["parryActiveColorRGB"] = COLORREF_to_JSON(m_parryActiveColorref);
  obj["parryInactiveColorRGB"] = COLORREF_to_JSON(m_parryInactiveColorref);

  SAVE_KEY_FIELD_CTOR(showText);
  SAVE_KEY_FIELD_CTOR(topmostWindow);
  SAVE_KEY_FIELD_CTOR(windowLeft);
  SAVE_KEY_FIELD_CTOR(windowTop);
  SAVE_KEY_FIELD_CTOR(windowWidth);
  SAVE_KEY_FIELD_CTOR(windowHeight);
  SAVE_KEY_FIELD_CTOR(pollingIntervalMS);
  SAVE_KEY_FIELD_CTOR(controllerID);

  obj["analogThresholds"] = m_analogThresholds.saveToJSON();
  obj["parryTimer"] = m_parryTimer.saveToJSON();
  obj["layoutParams"] = m_layoutParams.saveToJSON();

  return obj;
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

  loadFromJSON(obj);

  return "";
}


std::string GPVConfig::saveToFile(std::string const &fname) const
{
  JSON obj = saveToJSON();

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
