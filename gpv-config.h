// gpv-config.h
// `GPVConfig`, configuration for the gamepad viewer.

// See license.txt for copyright and terms of use.

#ifndef GPV_CONFIG_H
#define GPV_CONFIG_H

#include "json-fwd.h"                  // json::JSON

#include <windows.h>                   // COLORREF

#include <string>                      // std::string


// Configuration of analog input thresholds
class AnalogThresholdConfig {
public:      // data
  // When the trigger is greater than or equal to this value, we regard it
  // as "active".
  //
  // Substitutes for XINPUT_GAMEPAD_TRIGGER_THRESHOLD.
  //
  int m_triggerDeadZone;

  // When either axis of the right stick exceeds this value, it is treated
  // as active.
  //
  // Substitutes for XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE.
  //
  int m_rightStickDeadZone;

  // When the left stick exceeds the octagon with this as its radius, it
  // is treated as at least walking speed.
  //
  // Substitutes for XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE.
  //
  int m_leftStickWalkThreshold;

  // When the left stick exceeds a circle with this radius, the character
  // will run when on foot, and Torrent will gallop.
  int m_leftStickRunThreshold;

  // When the left stick exceeds a circle with this radius, Torrent will
  // maintain his sprint speed.
  int m_leftStickSprintThreshold;

public:      // methods
  AnalogThresholdConfig();

  // De/serialize as JSON.
  void loadFromJSON(json::JSON const &obj);
  json::JSON saveToJSON() const;
};


// User configuration settings for the gamepad viewer.
class GPVConfig {
public:      // data
  // Color to use to draw the lines.
  COLORREF m_linesColorref;

  // Color to use to draw the highlights.
  COLORREF m_highlightColorref;

  // If true, show the textual display of the controller inputs.
  bool m_showText;

  // If true, set our window to be on top of all others (that are not
  // also topmost).
  bool m_topmostWindow;

  // Window dimensions.
  int m_windowLeft;
  int m_windowTop;
  int m_windowWidth;
  int m_windowHeight;

  // Milliseconds between attempts to poll the controller.
  int m_pollingIntervalMS;

  // ID in [0,3] of the controller to poll.
  int m_controllerID;

  // Analog input thresholds.
  AnalogThresholdConfig m_analogThresholds;

public:      // methods
  // Initialize with defaults.
  GPVConfig();

  // De/serialize as JSON.
  void loadFromJSON(json::JSON const &obj);
  json::JSON saveToJSON() const;

  // Load settings from the named file.  Return an empty string on
  // success, and an error message otherwise.
  std::string loadFromFile(std::string const &fname);

  // Save the settings.  Return a non-empty error message on failure.
  std::string saveToFile(std::string const &fname) const;
};


#endif // GPV_CONFIG_H
