// gamepad-viewer.cc
// Program to show the current gamepad input state.

#include "gamepad-viewer.h"            // this module

#include "winapi-util.h"               // getLastErrorMessage, CreateWindowExWArgs

#include <d2d1.h>                      // Direct2D
#include <dwrite.h>                    // DirectWrite
#include <windows.h>                   // Windows API
#include <windowsx.h>                  // GET_X_PARAM, GET_Y_LPARAM

#include <algorithm>                   // std::min
#include <cassert>                     // assert
#include <cstdlib>                     // std::{getenv, atoi}
#include <cstring>                     // std::memset
#include <iostream>                    // std::{wcerr, flush}


// Level of diagnostics to print.
//
//   1: API call failures.
//
//   2: Information about messages, etc., of a moderate volume.
//
// The default value is not used, as `wWinMain` overwrites it.
//
int g_tracingLevel = 1;


// True to use the transparency effects.
//
// The default value is not used, as `wWinMain` overwrites it.
//
bool g_useTransparency = true;


// Write a diagnostic message.
#define TRACE(level, msg)           \
  if (g_tracingLevel >= (level)) {  \
    std::wcerr << msg << std::endl; \
  }

#define TRACE1(msg) TRACE(1, msg)
#define TRACE2(msg) TRACE(2, msg)

// Add the value of `expr` to a chain of outputs using `<<`.
#define TRVAL(expr) L" " WIDE_STRINGIZE(expr) L"=" << (expr)


GVMainWindow::GVMainWindow()
  : m_d2dFactory(nullptr),
    m_writeFactory(nullptr),
    m_textFormat(nullptr),
    m_renderTarget(nullptr),
    m_yellowBrush(nullptr),
    m_textBrush(nullptr),
    m_ellipse(),
    m_rotDegrees(0),
    m_controllerState()
{
  // I'm not sure if the default ctor initializes this.
  std::memset(&m_controllerState, 0, sizeof(m_controllerState));
}


void GVMainWindow::createDeviceIndependentResources()
{
  CALL_HR_WINAPI(D2D1CreateFactory,
    D2D1_FACTORY_TYPE_SINGLE_THREADED,
    &m_d2dFactory);
  assert(m_d2dFactory);

  CALL_HR_WINAPI(DWriteCreateFactory,
    DWRITE_FACTORY_TYPE_SHARED,
    __uuidof(m_writeFactory),
    reinterpret_cast<IUnknown **>(&m_writeFactory));
  assert(m_writeFactory);

  CALL_HR_WINAPI(m_writeFactory->CreateTextFormat,
    L"Verdana",                        // fontFamilyName
    nullptr,                           // fontCollection
    DWRITE_FONT_WEIGHT_NORMAL,         // fontWeight
    DWRITE_FONT_STYLE_NORMAL,          // fontStyle
    DWRITE_FONT_STRETCH_NORMAL,        // fontStretch
    50.0f,                             // fontSize
    L"",                               // localeName
    &m_textFormat                      // textFormat
  );
  assert(m_textFormat);

  // Center text horizontally and vertically.
  CALL_HR_WINAPI(m_textFormat->SetTextAlignment,
    DWRITE_TEXT_ALIGNMENT_CENTER);
  CALL_HR_WINAPI(m_textFormat->SetParagraphAlignment,
    DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}


void GVMainWindow::destroyDeviceIndependentResources()
{
  safeRelease(m_d2dFactory);
  safeRelease(m_writeFactory);
  safeRelease(m_textFormat);
}


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


void GVMainWindow::createGraphicsResources()
{
  if (!m_renderTarget) {
    D2D1_SIZE_U size = getClientRectSizeU();
    TRACE2(L"createGraphicsResources: size=(" << size.width <<
           L"x" << size.height << L")");

    CALL_HR_WINAPI(m_d2dFactory->CreateHwndRenderTarget,
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(m_hwnd, size),
      &m_renderTarget);
    assert(m_renderTarget);

    // Create a yellow brush used to fill the ellipse.
    D2D1_COLOR_F const color = D2D1::ColorF(1.0f, 1.0f, 0.0f);
    CALL_HR_WINAPI(m_renderTarget->CreateSolidColorBrush,
      color, &m_yellowBrush);
    assert(m_yellowBrush);

    CALL_HR_WINAPI(m_renderTarget->CreateSolidColorBrush,
      D2D1::ColorF(D2D1::ColorF::Blue),
      &m_textBrush);
    assert(m_textBrush);

    calculateLayout();
  }
}


void GVMainWindow::discardGraphicsResources()
{
  safeRelease(m_renderTarget);
  safeRelease(m_yellowBrush);
  safeRelease(m_textBrush);
}


void GVMainWindow::onPaint()
{
  createGraphicsResources();

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

  // Rotate the ellipse.
  m_renderTarget->SetTransform(
    D2D1::Matrix3x2F::Rotation(m_rotDegrees, m_ellipse.point));

  m_renderTarget->FillEllipse(m_ellipse, m_yellowBrush);

  // Draw some text in the ellipse.
  {
    static wchar_t const helloWorld[] = L"Hello, World!";
    D2D1_SIZE_F renderTargetSize = m_renderTarget->GetSize();

    m_renderTarget->DrawText(
      helloWorld,
      ARRAYSIZE(helloWorld) - 1,
      m_textFormat,
      D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height),
      m_textBrush);
  }

  HRESULT hr = m_renderTarget->EndDraw();
  if (hr == HRESULT(D2DERR_RECREATE_TARGET)) {
    // This is a normal condition (but `FAILED(hr)` is still true) that
    // means the target device has become invalid.  Dispose of resources
    // and prepare to re-create them.
    TRACE2(L"createGraphicsResources: D2DERR_RECREATE_TARGET");
    discardGraphicsResources();
  }
  else if (FAILED(hr)) {
    winapiDieHR(L"EndDraw", hr);
  }

  EndPaint(m_hwnd, &ps);
}


void GVMainWindow::onResize()
{
  if (m_renderTarget) {
    m_renderTarget->Resize(getClientRectSizeU());
    calculateLayout();

    // Cause a repaint event for the entire window, not just any newly
    // exposed part, because the size affects everything displayed.
    invalidateAllPixels();
  }
}


void GVMainWindow::invalidateAllPixels()
{
  InvalidateRect(m_hwnd, nullptr /*lpRect*/, false /*bErase*/);
}


bool GVMainWindow::onKeyDown(WPARAM wParam, LPARAM lParam)
{
  switch (wParam) {
    case 'Q':
      // Q to quit.
      TRACE2(L"Saw Q keypress.");
      PostMessage(m_hwnd, WM_CLOSE, 0, 0);
      return true;

    case VK_LEFT:
      m_rotDegrees -= 10;
      invalidateAllPixels();
      return true;

    case VK_RIGHT:
      m_rotDegrees += 10;
      invalidateAllPixels();
      return true;
  }

  // Not handled.
  return false;
}


LRESULT CALLBACK GVMainWindow::handleMessage(
  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
    case WM_CREATE: {
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

      createDeviceIndependentResources();
      return 0;
    }

    case WM_DESTROY:
      TRACE2(L"received WM_DESTROY");
      discardGraphicsResources();
      destroyDeviceIndependentResources();
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

    case WM_KEYDOWN:
      if (onKeyDown(wParam, lParam)) {
        // Handled.
        return 0;
      }
      break;
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

  // Configure tracing level, with default of 1.
  g_tracingLevel = envIntOr("TRACE", 1);

  // Configure transparency, with default of true.
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
