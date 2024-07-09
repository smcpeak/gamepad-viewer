// base-window.h
// `BaseWindow`, a base class for managing window state.

// Loosely based on
// https://learn.microsoft.com/en-us/windows/win32/learnwin32/managing-application-state-,
// except using an ordinary class instead of a template.

#ifndef BASE_WINDOW_H
#define BASE_WINDOW_H

#include "winapi-util.h"               // CreateWindowExWArgs


// This provides a virtual method to handle window messages.  Extending
// it provides a natural way to track per-window state.
class BaseWindow {
  // Instances of this class cannot be copied.
  BaseWindow(BaseWindow const &obj) = delete;
  BaseWindow &operator=(BaseWindow const &obj) = delete;

public:      // class data
  // Name of the window class.  A client could change this, but only
  // before any window is created.
  static wchar_t const *s_windowClassName;

  // True once the window class has been registered.
  static bool s_windowClassRegistered;

public:      // instance data
  // Window handle for this window.
  HWND m_hwnd;

public:      // methods
  virtual ~BaseWindow();

  // Initially this object is not associated with any window.  The
  // client must call `createWindow`.
  BaseWindow();

  // If `!s_windowClassRegistered`, register it.  This is called
  // automatically by `createWindow`.
  void registerWindowClassIfNecessary();

  // Create the window, setting `m_hwnd`.  Exit on failure.
  //
  // The `m_lpClassName` and `m_lpParam` fields of `cw` are ignored, as
  // this method provides its own values for them.
  //
  void createWindow(CreateWindowExWArgs const &cw);

  // Window procedure used by all `BaseWindow` instances.  It delegates
  // to `handleMessage`.
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  // Handle a window message, returning 0 if it is handled.  The default
  // implementation calls `DefWindowProc`.
  virtual LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


#endif // BASE_WINDOW_H
