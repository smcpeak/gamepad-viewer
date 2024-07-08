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


#endif // WINAPI_UTIL_H
