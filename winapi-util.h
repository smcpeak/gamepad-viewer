// winapi-util.h
// Various winapi utilities.

#ifndef WINAPI_UTIL_H
#define WINAPI_UTIL_H

#include <windows.h>                   // winapi

#include <string>                      // std::wstring


// Get the string corresponding to `errorCode`.  This string is a
// complete sentence, and does *not* end with a newline.
std::wstring getErrorMessage(DWORD errorCode);


// Get the string corresponding to `GetLastError()`.
std::wstring getLastErrorMessage();


// Given that `functionName` has failed, print an error message based on
// `GetLastError()` to stderr and exit(2).
void winapiDie(wchar_t const *functionName);


// Given that `functionName` has failed, but that function does not set
// `GetLastError()` ("NLE" stands for "No Last Error"), print an error
// message to stderr and exit(2).
void winapiDieNLE(wchar_t const *functionName);


// Structure to hold the arguments for a `CreateWindowExW` call.
class CreateWindowExWArgs {
public:      // data
  // Extended window style.  Initially 0.
  DWORD     m_dwExStyle;

  // Name of the window class.  Initially null.
  LPCWSTR   m_lpClassName;

  // Window text, used as the title for top-level window, text for
  // buttons, etc.  Initially null.
  LPCWSTR   m_lpWindowName;

  // Window style.  Initially 0.
  DWORD     m_dwStyle;

  // Initial window position and size.  Initially `CW_USEDEFAULT`.
  int       m_x;
  int       m_y;
  int       m_nWidth;
  int       m_nHeight;

  // Parent window.  Initially null.
  HWND      m_hwndParent;

  // Menu.  Initially null.
  HMENU     m_hMenu;

  // Instance handle.  Initially `GetModuleHandle(nullptr)`.
  HINSTANCE m_hInstance;

  // User data.  Initially null.
  LPVOID    m_lpParam;

public:      // methods
  CreateWindowExWArgs();

  // Pass the arguments to `CreateWindowExW`, returning whatever it
  // returns.
  HWND createWindow() const;
};


#endif // WINAPI_UTIL_H
