// winapi-util.h
// Various winapi utilities.

// See license.txt for copyright and terms of use.

#ifndef WINAPI_UTIL_H
#define WINAPI_UTIL_H

#include <windows.h>                   // winapi

#include <string>                      // std::{string, wstring}


// Stringize an argument as a wide string.
#define WIDE_STRINGIZE_HELPER(x) L ## x
#define WIDE_STRINGIZE(x) WIDE_STRINGIZE_HELPER(#x)


// Get the string corresponding to `errorCode`.  This string is a
// complete sentence, and does *not* end with a newline.
std::wstring getErrorMessage(DWORD errorCode);


// Get the string corresponding to `GetLastError()`.
std::wstring getLastErrorMessage();


// Get the string corresponding to `hr`.
std::wstring getHRErrorMessage(HRESULT hr);


// Given that `functionName` has failed, print an error message based on
// `GetLastError()` to stderr and exit(2).
void winapiDie(wchar_t const *functionName);


// Given that `functionName` has failed, but that function does not set
// `GetLastError()` ("NLE" stands for "No Last Error"), print an error
// message to stderr and exit(2).
void winapiDieNLE(wchar_t const *functionName);


// `functionName` failed with code `hr`.  Print an error and exit(2).
void winapiDieHR(wchar_t const *functionName, HRESULT hr);


// Call `function`, which returns an `HRESULT`, with the specified
// arguments.  Die if it fails.
#define CALL_HR_WINAPI(function, ...)                   \
  if (HRESULT hr = function(__VA_ARGS__); FAILED(hr)) { \
    winapiDieHR(WIDE_STRINGIZE(function), hr);          \
  }


// Convert from narrow string, assumed to use UTF-8 encoding, to wide
// string.
std::wstring toWideString(std::string const &str);


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


// If `ptr` is not null, call its `Release` method and nullify it.
template <typename T>
void safeRelease(T *&ptr)
{
  if (ptr) {
    ptr->Release();
    ptr = nullptr;
  }
}


#endif // WINAPI_UTIL_H
