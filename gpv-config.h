// gpv-config.h
// `GPVConfig`, configuration for the gamepad viewer.

// See license.txt for copyright and terms of use.

#ifndef GPV_CONFIG_H
#define GPV_CONFIG_H

#include <windows.h>                   // COLORREF

#include <string>                      // std::string


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

public:      // methods
  // Initialize with defaults.
  GPVConfig();

  // Load settings from the named file.  Return an empty string on
  // success, and an error message otherwise.
  std::string loadFromFile(std::string const &fname);

  // Save the settings.  Return a non-empty error message on failure.
  std::string saveToFile(std::string const &fname) const;
};


#endif // GPV_CONFIG_H
