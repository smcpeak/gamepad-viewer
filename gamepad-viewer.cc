// gamepad-viewer.cc
// Program to show the current gamepad input state.

#include "winapi-util.h"               // getLastErrorMessage, CreateWindowExWArgs

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
  CreateWindowExWArgs cw;
  cw.m_lpClassName = CLASS_NAME;
  cw.m_lpWindowName = L"Gamepad Viewer";
  cw.m_dwStyle = WS_OVERLAPPEDWINDOW;

  HWND hwnd = cw.createWindow();
  if (hwnd == NULL) {
    winapiDie(L"CreateWindowExW");
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
