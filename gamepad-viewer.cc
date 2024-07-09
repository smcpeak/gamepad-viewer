// gamepad-viewer.cc
// Program to show the current gamepad input state.

#include "gamepad-viewer.h"            // this module

#include "winapi-util.h"               // getLastErrorMessage, CreateWindowExWArgs

#include <d2d1.h>                      // Direct2D
#include <windows.h>                   // Windows API
#include <windowsx.h>                  // GET_X_PARAM, GET_Y_LPARAM

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


// True to use the transparency effects.
bool g_useTransparency = true;


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
    m_ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), x, y);

    TRACE2(L"calculateLayout:"
           L" width=" << size.width <<
           L" height=" << size.height <<
           L" x=" << x <<
           L" y=" << y);
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

    // Use a black background, which is then keyed as transparent.
    m_renderTarget->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f));

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

      if (g_useTransparency) {
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
          RGB(0,0,0),          // crKey, the transparent color.
          255,                 // bAlpha (ignored here I think).
          LWA_COLORKEY);       // dwFlags
        if (!ok) {
          winapiDie(L"SetLayeredWindowAttributes");
        }
      }

      return 0;
    }

    case WM_DESTROY:
      TRACE2(L"received WM_DESTROY");
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

    case WM_NCHITTEST: {
      // Arrange to drag the window whenever the ellipse is clicked, and
      // resize in the corner areas.
      //
      // https://stackoverflow.com/a/7773941/2659307
      //
      LRESULT hit = DefWindowProc(m_hwnd, uMsg, wParam, lParam);
      if (hit == HTCLIENT) {
        // Get client-relative click point.
        POINT clientRelPt;
        clientRelPt.x = GET_X_LPARAM(lParam);
        clientRelPt.y = GET_Y_LPARAM(lParam);
        if (!ScreenToClient(m_hwnd, &clientRelPt)) {
          winapiDie(L"ScreenToClient");
        }

        // Adjust the point to be relative to the center of the ellipse.
        D2D1_POINT_2F centerRelPt;
        centerRelPt.x = clientRelPt.x - m_ellipse.point.x;
        centerRelPt.y = clientRelPt.y - m_ellipse.point.y;

        // Declare out here so I can print them later.
        float scaledX = 0;
        float scaledY = 0;

        if (m_ellipse.radiusX > 0 && m_ellipse.radiusY > 0) {
          // Normalize the coordinate as if on a unit circle.
          scaledX = centerRelPt.x / m_ellipse.radiusX;
          scaledY = centerRelPt.y / m_ellipse.radiusY;

          if (scaledX*scaledX + scaledY*scaledY <= 0.5) {
            // The point is near the center of that circle.
            hit = HTCAPTION;           //  2
          }
          else {
            if (scaledX < 0) {
              if (scaledY < 0) {
                hit = HTTOPLEFT;       // 13
              }
              else {
                hit = HTBOTTOMLEFT;    // 16
              }
            }
            else {
              if (scaledY < 0) {
                hit = HTTOPRIGHT;      // 14
              }
              else {
                hit = HTBOTTOMRIGHT;   // 17
              }
            }
          }
        }

        #define LSTR(x) L ## x
        #define TRVAL(expr) L" " LSTR(#expr) L"=" << (expr)

        TRACE2(L"WM_NCHITTEST:"
          TRVAL(clientRelPt.x) <<
          TRVAL(clientRelPt.y) <<
          TRVAL(centerRelPt.x) <<
          TRVAL(centerRelPt.y) <<
          TRVAL(scaledX) <<
          TRVAL(scaledY) <<
          TRVAL(hit));
      }

      return hit;
    }

    case WM_KEYDOWN: {
      if (wParam == 'Q') {
        // Q to quit.
        TRACE2(L"Saw Q keypress.");
        PostMessage(m_hwnd, WM_CLOSE, 0, 0);
        return 0;
      }
      break;
    }
  }

  return BaseWindow::handleMessage(uMsg, wParam, lParam);
}


// If `envvar` is set, return its value as an integer.  Otherwise return
// `defaultValue`.
static int envIntOr(char const *envvar, int defaultValue)
{
  if (char const *value = std::getenv(envvar)) {
    return std::atoi(value);
  }
  else {
    return defaultValue;
  }
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow)
{
  // Elsewhere, I rely on the assumption that I can get the module
  // handle using `GetModuleHandle`.
  assert(hInstance == GetModuleHandle(nullptr));

  g_tracingLevel = envIntOr("TRACE", 1);
  g_useTransparency = envIntOr("TRANSPARENT", 1) != 0;

  // Create the window.
  CreateWindowExWArgs cw;
  if (g_useTransparency) {
    cw.m_dwExStyle = WS_EX_LAYERED;    // For `SetLayeredWindowAttributes`.
  }
  cw.m_lpWindowName = L"Gamepad Viewer";
  cw.m_x = cw.m_y = 300;
  cw.m_nWidth = cw.m_nHeight = 400;
  cw.m_dwStyle = WS_POPUP;

  GVMainWindow mainWindow;
  mainWindow.createWindow(cw);

  TRACE2(L"Calling ShowWindow");
  ShowWindow(mainWindow.m_hwnd, nCmdShow);

  // Run the message loop.

  MSG msg = { };
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  TRACE2(L"Returning from main");
  return 0;
}


// EOF
