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

  bool operator==(AnalogThresholdConfig const &obj) const;
  bool operator!=(AnalogThresholdConfig const &obj) const
    { return !operator==(obj); }

  // De/serialize as JSON.
  void loadFromJSON(json::JSON const &obj);
  json::JSON saveToJSON() const;
};


// Parameters for a button timer.
class ButtonTimerConfig {
public:      // data
  // Total duration, after which the timer expires.
  //
  // This can be set to zero to disable the timer display.
  int m_durationMS = 0;

  // Startup time before the active window begins.
  int m_activeStartMS = 0;

  // Time from start to the end of the active window.
  int m_activeEndMS = 0;

public:
  ButtonTimerConfig();

  bool operator==(ButtonTimerConfig const &obj) const;
  bool operator!=(ButtonTimerConfig const &obj) const
    { return !operator==(obj); }

  // De/serialize as JSON.
  void loadFromJSON(json::JSON const &obj);
  json::JSON saveToJSON() const;
};


// Parameters for the dodge invulnerability window timer.
class DodgeInvulnerabilityTimerConfig : public ButtonTimerConfig {
public:      // methods
  // Set defaults for dodging.
  DodgeInvulnerabilityTimerConfig();
};


// Parameters related to the parry timer.
class ParryTimerConfig : public ButtonTimerConfig {
public:      // data
  // Number of segments in the timer bar.
  int m_numSegments = 20;

  // If true, interpret the elapsed time as a number of frames early,
  // late, or within the active parry window.
  bool m_showAccuracy = false;

  // If true, show the elapsed time in milliseconds as text too.
  bool m_showElapsedTime = false;

public:      // methods
  ParryTimerConfig();

  bool operator==(ParryTimerConfig const &obj) const;
  bool operator!=(ParryTimerConfig const &obj) const
    { return !operator==(obj); }

  // De/serialize as JSON.
  void loadFromJSON(json::JSON const &obj);
  json::JSON saveToJSON() const;
};


// Parameters that control how the controller UI is laid out.
//
// All of these are in [0,1], representing fractional distances of the
// whole within either the whole UI or a parent button cluster.
//
class LayoutParams {
public:      // data
  // Font size for the text display in "device-independent pixel" units,
  // one of which is 1/96th of an inch.
  float m_textFontSizeDIPs = 16.0;

  // Distance from top to center of face button cluster and center of
  // select/start cluster.
  float m_faceButtonsY = 0.42;

  // Radius of face button clusters.
  float m_faceButtonsR = 0.15;

  // Radius of one of the round face buttons.
  float m_roundButtonR = 0.20;

  // If a just-released timer is shown inside a round button, its size
  // is this much times the size of the circle it is inside.
  float m_roundButtonTimerSizeFactor = 0.20;

  // Square radius of one of the dpad buttons.
  float m_dpadButtonR = 0.15;

  // Distance from side to center of shoulder buttons.
  float m_shoulderButtonsX = 0.15;

  // Radius of shoulder button cluster.
  float m_shoulderButtonsR = 0.125;

  // Vertical radius of a bumper button within its shoulder cluster.
  float m_bumperVR = 0.15;

  // Vertical radius of a trigger box within its shoulder cluster.
  float m_triggerVR = 0.35;

  // X/Y of center of parry timer.
  float m_parryTimerX = 0.5;
  float m_parryTimerY = 0.125;

  // H/V radius of parry timer.
  float m_parryTimerHR = 0.2;
  float m_parryTimerVR = 0.04;

  // Height of hash marks as a proportion of the meter height.
  float m_parryTimerHashHeight = 0.25;

  // Location of the top-left corner of the elapsed parry time text, in
  // proportional units relative to the parry timer region.  Thus, (0,1)
  // represents the bottom-left corner of that region.  This is only
  // shown if `ParryTimerConfig::m_showElapsedTime` is true.
  float m_parryElapsedTimeX = 0;
  float m_parryElapsedTimeY = 1;

  // Location of top-left corner of dodge timer text, relative to the
  // entire gamepad viewer display.
  float m_dodgeInvulnerabilityTimeX = 0.65;
  float m_dodgeInvulnerabilityTimeY = 0.6;

  // Radius of each stick display cluster.
  float m_stickR = 0.25;

  // Radius of the always-visible circle around the stick thumb.
  float m_stickOutlineR = 0.4;

  // Maximum distance of the thumb from its center.
  float m_stickMaxDeflectR = 0.3;

  // Radius of the filled circle representing the thumb.
  float m_stickThumbR = 0.1;

  // By how much vertical space are the chevrons separated?
  float m_chevronSeparation = 0.2;

  // Horizontal radius of the chevrons.
  float m_chevronHR = 0.25;

  // Vertical radius of the chevrons.
  float m_chevronVR = 0.17;

  // Horizontal distance from the center line to the sel/start buttons.
  float m_selStartX = 0.08;

  // Horizontal and vertical radii for sel/start.
  float m_selStartHR = 0.05;
  float m_selStartVR = 0.03;

  // Distance from the top to the central circle.
  float m_centralCircleY = 0.52;

  // Radius of the central circle.
  float m_centralCircleR = 0.035;

  // Distance that most uses of `drawCircle` and `drawSquare` leave
  // between the edge of the circle and the edge of its nominal area.
  float m_circleMargin = 0.1;

  // Width in pixels of the lines.
  float m_lineWidthPixels = 3.0;

public:      // methods
  LayoutParams();

  bool operator==(LayoutParams const &obj) const;
  bool operator!=(LayoutParams const &obj) const
    { return !operator==(obj); }

  // De/serialize as JSON.
  void loadFromJSON(json::JSON const &obj);
  json::JSON saveToJSON() const;
};


// User configuration settings for the gamepad viewer.
class GPVConfig {
public:      // data
  // NOTE: None of the colors can be black, because black is used as the
  // transparency key color (and I cannot easily change that due to a
  // bug in how Windows interprets transparency).

  // Color to use to draw the lines.
  COLORREF m_linesColorref;

  // Color to use to draw the highlights.
  COLORREF m_highlightColorref;

  // Colors for active and inactive parry.
  COLORREF m_parryActiveColorref;
  COLORREF m_parryInactiveColorref;

  // Color for text background.
  COLORREF m_textBackgroundColorref;

  // Colors for active and inactive dodge.
  COLORREF m_dodgeActiveColorref;
  COLORREF m_dodgeInactiveColorref;

  // If true, show the textual display of the controller inputs.
  bool m_showText;

  // True to show the invulnerability timer frame data.
  bool m_showDodgeInvulnerabilityTimer;

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

  // Milliseconds after dodge button is released for which we should
  // show a small dot inside the circle.  Zero disables that display.
  int m_dodgeReleaseTimerDurationMS;

  // ID in [0,3] of the controller to poll.
  int m_controllerID;

  // Analog input thresholds.
  AnalogThresholdConfig m_analogThresholds;

  // Dodge invulnerability timer configuration.
  DodgeInvulnerabilityTimerConfig m_dodgeInvulnerabilityTimer;

  // Parry timer configuration.
  ParryTimerConfig m_parryTimer;

  // UI layout.
  LayoutParams m_layoutParams;

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
