// winapi-util.cc
// Code for `winapi-util` module.

#include "winapi-util.h"               // this module

#include <windows.h>                   // winapi

#include <string>                      // std::wstring


// This is copied+modified from smbase/syserr.cc.
std::wstring getErrorMessage(DWORD errorCode)
{
  // Get the string for `errorCode`.
  LPVOID lpMsgBuf;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    errorCode,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL
  );

  // Now the SDK says: "Process any inserts in lpMsgBuf."
  //
  // I think this means that lpMsgBuf might have "%" escape
  // sequences in it... oh well, I'm just going to keep them.

  // Make a copy of the string.
  std::wstring ret((wchar_t*)lpMsgBuf);

  // Free the buffer.
  LocalFree(lpMsgBuf);

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


// EOF
