// winapi-util.cc
// Code for `winapi-util` module.

#include "winapi-util.h"               // this module

#include <windows.h>                   // winapi

#include <cstdlib>                     // std::exit
#include <iostream>                    // std::wcerr
#include <sstream>                     // std::wostreamstream
#include <string>                      // std::wstring


// This is copied+modified from smbase/syserr.cc.
std::wstring getErrorMessage(DWORD errorCode)
{
  // Get the string for `errorCode`.
  LPVOID lpMsgBuf;
  if (!FormatMessage(
         FORMAT_MESSAGE_ALLOCATE_BUFFER |
         FORMAT_MESSAGE_FROM_SYSTEM |
         FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL,
         errorCode,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
         (LPTSTR) &lpMsgBuf,
         0,
         NULL)) {
    std::wcerr << L"FormatMessage failed with code "
               << GetLastError() << L"\n";
    std::exit(4);
  }

  // Now the SDK says: "Process any inserts in lpMsgBuf."
  //
  // I think this means that lpMsgBuf might have "%" escape
  // sequences in it... oh well, I'm just going to keep them.

  // Make a copy of the string.
  std::wstring ret((wchar_t*)lpMsgBuf);

  // Free the buffer.
  if (LocalFree(lpMsgBuf) != NULL) {
    winapiDie(L"LocalFree");
  }

  // At least some error messages end with a newline, but I do not want
  // that.
  if (!ret.empty() && ret[ret.size()-1] == L'\n') {
    ret = ret.substr(0, ret.size()-1);
  }

  return ret;
}


std::wstring getLastErrorMessage()
{
  return getErrorMessage(GetLastError());
}


std::wstring getHRErrorMessage(HRESULT hr)
{
  DWORD facility = HRESULT_FACILITY(hr);
  DWORD code = HRESULT_CODE(hr);

  if (facility == FACILITY_WIN32) {
    return getErrorMessage(code);
  }
  else {
    std::wostringstream oss;
    oss << std::hex;
    oss << L"HRESULT error: facility=" << facility
        << L" code=" << code << L".";
    return oss.str();
  }
}


void winapiDie(wchar_t const *functionName)
{
  DWORD code = GetLastError();
  std::wcerr << functionName << L": " << getErrorMessage(code) << L"\n";
  std::exit(2);
}


void winapiDieNLE(wchar_t const *functionName)
{
  std::wcerr << functionName << L" failed.\n";
  std::exit(2);
}


void winapiDieHR(wchar_t const *functionName, HRESULT hr)
{
  std::wcerr << functionName << L": " << getHRErrorMessage(hr) << L"\n";
  std::exit(2);
}


// ----------------------- CreateWindowExWArgs -------------------------
CreateWindowExWArgs::CreateWindowExWArgs()
  : m_dwExStyle(0),
    m_lpClassName(nullptr),
    m_lpWindowName(nullptr),
    m_dwStyle(0),
    m_x(CW_USEDEFAULT),
    m_y(CW_USEDEFAULT),
    m_nWidth(CW_USEDEFAULT),
    m_nHeight(CW_USEDEFAULT),
    m_hwndParent(nullptr),
    m_hMenu(nullptr),
    m_hInstance(GetModuleHandle(nullptr)),
    m_lpParam(nullptr)
{}


HWND CreateWindowExWArgs::createWindow() const
{
  return CreateWindowExW(
    m_dwExStyle,
    m_lpClassName,
    m_lpWindowName,
    m_dwStyle,
    m_x,
    m_y,
    m_nWidth,
    m_nHeight,
    m_hwndParent,
    m_hMenu,
    m_hInstance,
    m_lpParam);
}


// EOF
