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


// If true, save all values.  Otherwise, only save values that are set
// to something other than the default value.
//
// The false value was an experiment that didn't work as well as I'd
// hoped.
//
bool const saveAllValues = true;


// True if `name` of `this` equals the same field in `obj`.
#define EMEMB(name) (name == obj.name)


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


// For when the data type is `bool`.
#define LOAD_KEY_BOOL_FIELD(name) \
  LOAD_FIELD(#name, m_##name, data.ToBool())

// For when the data type is `int`.
#define LOAD_KEY_INT_FIELD(name) \
  LOAD_FIELD(#name, m_##name, data.ToInt())

// For when the data type is `float`.
#define LOAD_KEY_FLOAT_FIELD(name) \
  LOAD_FIELD(#name, m_##name, data.ToFloat())


// Save a field where converting to JSON only requires invoking one of
// the `JSON` constructors.  But only if the value is different from the
// default.
#define SAVE_KEY_FIELD_CTOR(name)                     \
  if (saveAllValues || (m_##name != dflt.m_##name)) { \
    obj[#name] = JSON(m_##name);                      \
  }


// ----------------------- AnalogThresholdConfig -----------------------
AnalogThresholdConfig::AnalogThresholdConfig()
    // These defaults are tuned for Elden Ring.
  : m_triggerDeadZone(127),
    m_rightStickDeadZone(6600),
    m_leftStickWalkThreshold(16000),
    m_leftStickRunThreshold(25500),
    m_leftStickSprintThreshold(30000)
{}


#define X_ATC_FIELDS          \
  X(triggerDeadZone)          \
  X(rightStickDeadZone)       \
  X(leftStickWalkThreshold)   \
  X(leftStickRunThreshold)    \
  X(leftStickSprintThreshold)


bool AnalogThresholdConfig::operator==(
  AnalogThresholdConfig const &obj) const
{
  #define X(name) EMEMB(m_##name) &&

  return X_ATC_FIELDS
         true;

  #undef X
}


void AnalogThresholdConfig::loadFromJSON(JSON const &obj)
{
  #define X(name) \
    LOAD_KEY_FIELD(name, data.ToInt());

  X_ATC_FIELDS

  #undef X
}


JSON AnalogThresholdConfig::saveToJSON() const
{
  JSON obj = json::Object();
  AnalogThresholdConfig dflt;

  #define X(name) \
    SAVE_KEY_FIELD_CTOR(name);

  X_ATC_FIELDS

  #undef X

  return obj;
}


#undef X_ATC_FIELDS


// ------------------------- ParryTimerConfig --------------------------
ParryTimerConfig::ParryTimerConfig()
  // Defaults in class body.
{}


#define X_PTC_FIELDS       \
  X(durationMS,      INT)  \
  X(numSegments,     INT)  \
  X(activeStartMS,   INT)  \
  X(activeEndMS,     INT)  \
  X(showElapsedTime, BOOL)


bool ParryTimerConfig::isActive(int elapsedMS) const
{
  return m_activeStartMS <= elapsedMS &&
                            elapsedMS <= m_activeEndMS;
}


bool ParryTimerConfig::operator==(ParryTimerConfig const &obj) const
{
  #define X(name, TYPE) EMEMB(m_##name) &&

  return X_PTC_FIELDS
         true;

  #undef X
}


void ParryTimerConfig::loadFromJSON(json::JSON const &obj)
{
  #define X(name, TYPE) \
    LOAD_KEY_##TYPE##_FIELD(name);

  X_PTC_FIELDS

  #undef X
}


json::JSON ParryTimerConfig::saveToJSON() const
{
  JSON obj = json::Object();
  ParryTimerConfig dflt;

  #define X(name, TYPE) \
    SAVE_KEY_FIELD_CTOR(name);

  X_PTC_FIELDS

  #undef X

  return obj;
}


#undef X_PTC_FIELDS


// --------------------------- LayoutParams ----------------------------
LayoutParams::LayoutParams()
  // All of the default values are in the class body.
{}


#define X_LP_FIELDS             \
  X(textFontSizeDIPs)           \
  X(faceButtonsY)               \
  X(faceButtonsR)               \
  X(roundButtonR)               \
  X(roundButtonTimerSizeFactor) \
  X(dpadButtonR)                \
  X(shoulderButtonsX)           \
  X(shoulderButtonsR)           \
  X(bumperVR)                   \
  X(triggerVR)                  \
  X(parryTimerX)                \
  X(parryTimerY)                \
  X(parryTimerHR)               \
  X(parryTimerVR)               \
  X(parryTimerHashHeight)       \
  X(parryElapsedTimeX)          \
  X(parryElapsedTimeY)          \
  X(stickR)                     \
  X(stickOutlineR)              \
  X(stickMaxDeflectR)           \
  X(stickThumbR)                \
  X(chevronSeparation)          \
  X(chevronHR)                  \
  X(chevronVR)                  \
  X(selStartX)                  \
  X(selStartHR)                 \
  X(selStartVR)                 \
  X(centralCircleY)             \
  X(centralCircleR)             \
  X(circleMargin)               \
  X(lineWidthPixels)


bool LayoutParams::operator==(LayoutParams const &obj) const
{
  #define X(name) EMEMB(m_##name) &&

  return X_LP_FIELDS
         true;

  #undef X
}


void LayoutParams::loadFromJSON(JSON const &obj)
{
  #define X(name) \
    LOAD_KEY_FLOAT_FIELD(name);

  X_LP_FIELDS

  #undef X
}


JSON LayoutParams::saveToJSON() const
{
  JSON obj = json::Object();
  LayoutParams dflt;

  #define X(name) \
    SAVE_KEY_FIELD_CTOR(name);

  X_LP_FIELDS

  #undef X

  return obj;
}


#undef X_LP_FIELDS


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
    m_dodgeTimerDurationMS(33),                  // 1 frame at 30 FPS.
    m_controllerID(0),                           // First controller.
    m_analogThresholds(),
    m_parryTimer(),
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


// Not used at the moment, but retaining this in case I want to later.
#define X_GPVC_FIELDS         \
  X_COLOR(linesColor)         \
  X_COLOR(highlightColor)     \
  X_COLOR(parryActiveColor)   \
  X_COLOR(parryInactiveColor) \
  X_BOOL(showText)            \
  X_BOOL(topmostWindow)       \
  X_INT(windowLeft)           \
  X_INT(windowTop)            \
  X_INT(windowWidth)          \
  X_INT(windowHeight)         \
  X_INT(pollingIntervalMS)    \
  X_INT(dodgeTimerDurationMS) \
  X_INT(controllerID)         \
  X_OBJ(analogThresholds)     \
  X_OBJ(parryTimer)           \
  X_OBJ(layoutParams)


void GPVConfig::loadFromJSON(JSON const &obj)
{
  #define LOAD_KEY_FIELD_COLOR(name) \
    LOAD_FIELD(#name "RGB", m_##name##ref, COLORREF_from_JSON(data));

  LOAD_KEY_FIELD_COLOR(linesColor)
  LOAD_KEY_FIELD_COLOR(highlightColor)
  LOAD_KEY_FIELD_COLOR(parryActiveColor)
  LOAD_KEY_FIELD_COLOR(parryInactiveColor)

  #undef LOAD_KEY_FIELD_COLOR

  LOAD_KEY_FIELD(showText, data.ToBool());
  LOAD_KEY_FIELD(topmostWindow, data.ToBool());
  LOAD_KEY_FIELD(windowLeft, data.ToInt());
  LOAD_KEY_FIELD(windowTop, data.ToInt());
  LOAD_KEY_FIELD(windowWidth, data.ToInt());
  LOAD_KEY_FIELD(windowHeight, data.ToInt());
  LOAD_KEY_FIELD(pollingIntervalMS, data.ToInt());
  LOAD_KEY_FIELD(dodgeTimerDurationMS, data.ToInt());
  LOAD_KEY_FIELD(controllerID, data.ToInt());

  #define LOAD_KEY_FIELD_OBJ(name)          \
    if (obj.hasKey(#name)) {                \
      m_##name.loadFromJSON(obj.at(#name)); \
    }

  LOAD_KEY_FIELD_OBJ(analogThresholds)
  LOAD_KEY_FIELD_OBJ(parryTimer)
  LOAD_KEY_FIELD_OBJ(layoutParams)

  #undef LOAD_KEY_FIELD_OBJ
}


JSON GPVConfig::saveToJSON() const
{
  JSON obj = json::Object();
  GPVConfig dflt;

  #define SAVE_KEY_FIELD_COLOR(name)                              \
    if (saveAllValues || (m_##name##ref != dflt.m_##name##ref)) { \
      obj[#name "RGB"] = COLORREF_to_JSON(m_##name##ref);         \
    }

  SAVE_KEY_FIELD_COLOR(linesColor)
  SAVE_KEY_FIELD_COLOR(highlightColor)
  SAVE_KEY_FIELD_COLOR(parryActiveColor)
  SAVE_KEY_FIELD_COLOR(parryInactiveColor)

  #undef SAVE_KEY_FIELD_COLOR

  SAVE_KEY_FIELD_CTOR(showText);
  SAVE_KEY_FIELD_CTOR(topmostWindow);
  SAVE_KEY_FIELD_CTOR(windowLeft);
  SAVE_KEY_FIELD_CTOR(windowTop);
  SAVE_KEY_FIELD_CTOR(windowWidth);
  SAVE_KEY_FIELD_CTOR(windowHeight);
  SAVE_KEY_FIELD_CTOR(pollingIntervalMS);
  SAVE_KEY_FIELD_CTOR(dodgeTimerDurationMS);
  SAVE_KEY_FIELD_CTOR(controllerID);

  #define SAVE_KEY_FIELD_OBJ(name)                      \
    if (saveAllValues || (m_##name != dflt.m_##name)) { \
      obj[#name] = m_##name.saveToJSON();               \
    }

  SAVE_KEY_FIELD_OBJ(analogThresholds)
  SAVE_KEY_FIELD_OBJ(parryTimer)
  SAVE_KEY_FIELD_OBJ(layoutParams)

  #undef SAVE_KEY_FIELD_OBJ

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


#undef X_GPVC_FIELDS


// EOF
