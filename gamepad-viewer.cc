// gamepad-viewer.cc
// Program to show the current gamepad input state.

#include "gamepad-viewer.h"            // this module

#include "winapi-util.h"               // getLastErrorMessage, CreateWindowExWArgs

#include <d2d1.h>                      // Direct2D
#include <windows.h>                   // Windows API

#include <algorithm>                   // std::min
#include <cassert>                     // assert
#include <cstdlib>                     // std::{getenv, atoi}
#include <iostream>                    // std::{wcerr, flush}


// Level of diagnostics to print.
//
//   1: API call failures.
//
//   2: Information about messages, etc., of a moderate volume.
//
int g_tracingLevel = 1;


// Write a diagnostic message.
#define TRACE(level, msg)           \
  if (g_tracingLevel >= (level)) {  \
    std::wcerr << msg << std::endl; \
  }

#define TRACE1(msg) TRACE(1, msg)
#define TRACE2(msg) TRACE(2, msg)


D2D1_SIZE_U GVMainWindow::getClientRectSizeU() const
{
  RECT rc;
  GetClientRect(m_hwnd, &rc);

  return D2D1::SizeU(rc.right - rc.left,
                     rc.bottom - rc.top);
}


void GVMainWindow::calculateLayout()
{
  if (m_renderTarget) {
    D2D1_SIZE_F size = m_renderTarget->GetSize();
    float const x = size.width / 2;
    float const y = size.height / 2;
    float const radius = std::min(x, y);
    m_ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);

    TRACE2(L"calculateLayout:"
           L" width=" << size.width <<
           L" height=" << size.height <<
           L" x=" << x <<
           L" y=" << y <<
           L" radius=" << radius);
  }
}


HRESULT GVMainWindow::createGraphicsResources()
{
  HRESULT hr = S_OK;
  if (!m_renderTarget) {
    D2D1_SIZE_U size = getClientRectSizeU();
    TRACE2(L"createGraphicsResources: size=(" << size.width <<
           L"x" << size.height << L")");

    hr = m_d2dFactory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(m_hwnd, size),
      &m_renderTarget);

    if (SUCCEEDED(hr)) {
      assert(m_renderTarget);

      // Create a yellow brush used to fill the ellipse.
      D2D1_COLOR_F const color = D2D1::ColorF(1.0f, 1.0f, 0.0f);
      hr = m_renderTarget->CreateSolidColorBrush(color, &m_brush);

      if (SUCCEEDED(hr)) {
        assert(m_brush);
        calculateLayout();
      }
      else {
        TRACE1(L"CreateSolidColorBrush failed");
      }
    }
    else {
      TRACE1(L"CreateHwndRenderTarget failed");
    }
  }

  return hr;
}


void GVMainWindow::discardGraphicsResources()
{
  safeRelease(m_renderTarget);
  safeRelease(m_brush);
}


void GVMainWindow::onPaint()
{
  HRESULT hr = createGraphicsResources();
  if (SUCCEEDED(hr)) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);
    if (!hdc) {
      winapiDieNLE(L"BeginPaint");
    }
    // The `hdc` is not further used because this function uses D2D
    // rather than GDI.

    m_renderTarget->BeginDraw();

    // Use a purple background, which is then keyed as transparent.
    m_renderTarget->Clear(D2D1::ColorF(1.0f, 0.0f, 1.0f));

    m_renderTarget->FillEllipse(m_ellipse, m_brush);

    hr = m_renderTarget->EndDraw();
    if (FAILED(hr) || hr == HRESULT(D2DERR_RECREATE_TARGET)) {
      TRACE1(L"createGraphicsResources: EndDraw: failed=" <<
             bool(FAILED(hr)) << L" recreate=" <<
             bool(hr == HRESULT(D2DERR_RECREATE_TARGET)));
      discardGraphicsResources();
    }

    EndPaint(m_hwnd, &ps);
  }
}


void GVMainWindow::onResize()
{
  if (m_renderTarget) {
    m_renderTarget->Resize(getClientRectSizeU());
    calculateLayout();

    // Cause a repaint event for the entire window, not just any newly
    // exposed part, because the size affects everything displayed.
    InvalidateRect(m_hwnd, nullptr /*lpRect*/, false /*bErase*/);
  }
}


LRESULT CALLBACK GVMainWindow::handleMessage(
  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
    case WM_CREATE: {
      HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &m_d2dFactory);
      if (FAILED(hr)) {
        // Cause `CreateWindowEx` to fail.
        TRACE1(L"D2D1CreateFactory failed");
        return -1;
      }

      // Arrange to treat purple as transparent.
      //
      // The exact choice of transparent color is important!  In
      // particular, the red and blue values must be equal, otherwise
      // mouse clicks do or do not go through correctly.  See:
      //
      //   https://stackoverflow.com/a/35242134/2659307
      //
      // Even then, after a few interactions, the window decorations
      // stop being interactable until one minimizes and restores.
      //
      // Note: It does *not* work to use a background color with zero
      // alpha and then use `LWA_ALPHA`.  `LWA_ALPHA` just applies its
      // alpha to the entire window, while the alpha channel of the
      // color is ignored.
      //
      BOOL ok = SetLayeredWindowAttributes(
        m_hwnd,
        RGB(255,0,255),      // crKey, the transparent color.
        255,                 // bAlpha (ignored here I think).
        LWA_COLORKEY);       // dwFlags
      if (!ok) {
        winapiDie(L"SetLayeredWindowAttributes");
      }

      return 0;
    }

    case WM_DESTROY:
      discardGraphicsResources();
      safeRelease(m_d2dFactory);
      PostQuitMessage(0);
      return 0;

    case WM_PAINT:
      onPaint();
      return 0;

    case WM_SIZE:
      onResize();
      return 0;
  }

  return BaseWindow::handleMessage(uMsg, wParam, lParam);
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow)
{
  // Elsewhere, I rely on the assumption that I can get the module
  // handle using `GetModuleHandle`.
  assert(hInstance == GetModuleHandle(nullptr));

  // Set tracing level based on envvar.
  if (char const *traceStr = std::getenv("TRACE")) {
    g_tracingLevel = std::atoi(traceStr);
  }

  // Create the window.
  CreateWindowExWArgs cw;
  cw.m_dwExStyle = WS_EX_LAYERED;      // For `SetLayeredWindowAttributes`.
  cw.m_lpWindowName = L"Gamepad Viewer";
  cw.m_dwStyle = WS_OVERLAPPEDWINDOW;

  GVMainWindow mainWindow;
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
