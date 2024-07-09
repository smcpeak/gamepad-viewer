// gamepad-viewer.cc
// Program to show the current gamepad input state.

#include "base-window.h"               // BaseWindow
#include "winapi-util.h"               // getLastErrorMessage, CreateWindowExWArgs

#include <windows.h>                   // Windows API

#include <cassert>                     // assert


class MainWindow : public BaseWindow {
public:      // methods
  virtual LRESULT handleMessage(
    UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};


LRESULT CALLBACK MainWindow::handleMessage(
  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(m_hwnd, &ps);
      if (!hdc) {
        winapiDieNLE(L"BeginPaint");
      }

      // All painting occurs here, between BeginPaint and EndPaint.

      if (!FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1))) {
        winapiDieNLE(L"FillRect");
      }

      EndPaint(m_hwnd, &ps);
      return 0;
    }
  }

  return BaseWindow::handleMessage(uMsg, wParam, lParam);
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow)
{
  // Elsewhere, I rely on the assumption that I can get the module
  // handle using `GetModuleHandle`.
  assert(hInstance == GetModuleHandle(nullptr));

  // Create the window.
  CreateWindowExWArgs cw;
  cw.m_lpWindowName = L"Gamepad Viewer";
  cw.m_dwStyle = WS_OVERLAPPEDWINDOW;

  MainWindow mainWindow;
  mainWindow.createWindow(cw);

  ShowWindow(mainWindow.m_hwnd, nCmdShow);

  // Run the message loop.

  MSG msg = { };
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}


// EOF
