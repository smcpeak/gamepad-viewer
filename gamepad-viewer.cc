// gamepad-viewer.cc
// Program to show the current gamepad input state.

#include "winapi-util.h"               // getLastErrorMessage

#include <windows.h>                   // Windows API

#include <iostream>                    // std::wcout


LRESULT CALLBACK mainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      if (!hdc) {
        winapiDieNLE(L"BeginPaint");
      }

      // All painting occurs here, between BeginPaint and EndPaint.

      if (!FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1))) {
        winapiDieNLE(L"FillRect");
      }

      EndPaint(hwnd, &ps);
      return 0;
    }
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
  // Register the window class.
  const wchar_t CLASS_NAME[]  = L"Gamepad Viewer";

  WNDCLASS wc = { };

  wc.lpfnWndProc   = mainWindowProc;
  wc.hInstance     = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);

  if (RegisterClass(&wc) == 0) {
    winapiDie(L"RegisterClass");
  }

  // Create the window.

  HWND hwnd = CreateWindowEx(
    0,                              // Optional window styles.
    CLASS_NAME,                     // Window class.
    L"Gamepad Viewer",              // Window text (title).
    WS_OVERLAPPEDWINDOW,            // Window style.

    // Size and position
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

    NULL,       // Parent window
    NULL,       // Menu
    hInstance,  // Instance handle
    NULL        // Additional application data
  );

  if (hwnd == NULL) {
    winapiDie(L"CreateWindowEx");
    return 2;
  }

  ShowWindow(hwnd, nCmdShow);

  // Run the message loop.

  MSG msg = { };
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}


// EOF
