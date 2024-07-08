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


#endif // WINAPI_UTIL_H
