// gamepad-viewer.cc
// Program to show the current gamepad input state.

#include "gamepad-viewer.h"            // this module

#include "winapi-util.h"               // getLastErrorMessage, CreateWindowExWArgs

#include <d2d1.h>                      // Direct2D
#include <d2d1_1.h>                    // D2D1_STROKE_STYLE_PROPERTIES1
#include <dwrite.h>                    // DirectWrite
#include <windows.h>                   // Windows API
#include <windowsx.h>                  // GET_X_PARAM, GET_Y_LPARAM
#include <xinput.h>                    // XInputGetState

#include <algorithm>                   // std::min
#include <cassert>                     // assert
#include <cstdlib>                     // std::{getenv, atoi}
#include <cstring>                     // std::memset
#include <iostream>                    // std::{wcerr, flush}
#include <sstream>                     // std::wostringstream


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
    m_strokeStyleFixedThickness(nullptr),
    m_renderTarget(nullptr),
    m_yellowBrush(nullptr),
    m_textBrush(nullptr),
    m_linesBrush(nullptr),
    m_ellipse(),
    m_rotDegrees(0),
    m_controllerState(),
    m_hasControllerState(false)
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
    12.0f,                             // fontSize
    L"",                               // localeName
    &m_textFormat                      // textFormat
  );
  assert(m_textFormat);

  if (false) {
    // Center text horizontally and vertically.
    CALL_HR_WINAPI(m_textFormat->SetTextAlignment,
      DWRITE_TEXT_ALIGNMENT_CENTER);
    CALL_HR_WINAPI(m_textFormat->SetParagraphAlignment,
      DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }

  // Make a stroke style that has a fixed width, thereby avoiding the
  // effects of coordinate transformations.
  //
  // Everything here is intended to be the defaults, except for the
  // `transformType`.
  //
  // See https://stackoverflow.com/a/75570749/2659307 .
  //
  D2D1_STROKE_STYLE_PROPERTIES1 ssp{};
  ssp.startCap = D2D1_CAP_STYLE_FLAT;
  ssp.endCap = D2D1_CAP_STYLE_FLAT;
  ssp.dashCap = D2D1_CAP_STYLE_FLAT;
  ssp.lineJoin = D2D1_LINE_JOIN_MITER;
  ssp.miterLimit = 10.0f;
  ssp.dashStyle = D2D1_DASH_STYLE_SOLID;
  ssp.dashOffset = 0.0f;
  ssp.transformType = D2D1_STROKE_TRANSFORM_TYPE_FIXED;

  CALL_HR_WINAPI(m_d2dFactory->CreateStrokeStyle,
    ssp,
    nullptr,
    0,
    &m_strokeStyleFixedThickness);
  assert(m_strokeStyleFixedThickness);
}


void GVMainWindow::destroyDeviceIndependentResources()
{
  safeRelease(m_d2dFactory);
  safeRelease(m_writeFactory);
  safeRelease(m_textFormat);
  safeRelease(m_strokeStyleFixedThickness);
}


void GVMainWindow::pollControllerState()
{
  memset(&m_controllerState, 0, sizeof(m_controllerState));
  DWORD res = XInputGetState(0 /*index*/, &m_controllerState);
  m_hasControllerState = (res == ERROR_SUCCESS);
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
    D2D1_COLOR_F const color = D2D1::ColorF(0.9, 0.9, 0.75);
    CALL_HR_WINAPI(m_renderTarget->CreateSolidColorBrush,
      color, &m_yellowBrush);
    assert(m_yellowBrush);

    CALL_HR_WINAPI(m_renderTarget->CreateSolidColorBrush,
      D2D1::ColorF(D2D1::ColorF::Blue),
      &m_textBrush);
    assert(m_textBrush);

    CALL_HR_WINAPI(m_renderTarget->CreateSolidColorBrush,
      D2D1::ColorF(0.5, 0.5, 0.9),
      &m_linesBrush);

    calculateLayout();
  }
}


void GVMainWindow::discardGraphicsResources()
{
  safeRelease(m_renderTarget);
  safeRelease(m_yellowBrush);
  safeRelease(m_textBrush);
  safeRelease(m_linesBrush);
}


void GVMainWindow::onTimer(WPARAM wParam)
{
  DWORD prevPN = m_controllerState.dwPacketNumber;

  pollControllerState();

  if (prevPN != m_controllerState.dwPacketNumber) {
    // Redraw to show the new state.
    invalidateAllPixels();
  }
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

  drawControllerState();

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


// Create a transformation matrix so that (0,0) is mapped to
// (left,top) and (1,1) is mapped to (right,bottom).
static D2D1_MATRIX_3X2_F focusArea(
  float left,
  float top,
  float right,
  float bottom)
{
  return
    D2D1::Matrix3x2F::Scale(right-left, bottom-top) *
    D2D1::Matrix3x2F::Translation(left, top);
}


void GVMainWindow::drawControllerState()
{
  std::wostringstream oss;

  XINPUT_STATE const &i = m_controllerState;
  XINPUT_GAMEPAD const &g = i.Gamepad;

  oss << L"hasState: " << m_hasControllerState << L"\n";
  oss << L"packet: " << i.dwPacketNumber << L"\n";
  oss << L"buttons: " << std::hex << g.wButtons << std::dec << L"\n";
  oss << L"leftTrigger: " << +g.bLeftTrigger << L"\n";
  oss << L"rightTrigger: " << +g.bRightTrigger << L"\n";
  oss << L"thumbLX: " << g.sThumbLX << L"\n";
  oss << L"thumbLY: " << g.sThumbLY << L"\n";
  oss << L"thumbRX: " << g.sThumbRX << L"\n";
  oss << L"thumbRY: " << g.sThumbRY << L"\n";

  D2D1_SIZE_F renderTargetSize = m_renderTarget->GetSize();

  std::wstring s = oss.str();
  m_renderTarget->DrawText(
    s.data(),
    s.size(),
    m_textFormat,
    D2D1::RectF(150, 10, renderTargetSize.width, renderTargetSize.height),
    m_textBrush);

  if (!( renderTargetSize.width > 0 && renderTargetSize.height > 0 )) {
    // Bail if the sizes are zero.
    return;
  }

  // Create a coordinate system where the upper-left is (0,0) and the
  // lower-right is (1,1).
  D2D1_MATRIX_3X2_F baseTransform =
    D2D1::Matrix3x2F::Scale(renderTargetSize.width, renderTargetSize.height);

  // Draw a circle in the lower-left quadrant.
  drawCircle(focusArea(0.0, 0.5, 0.5, 1.0) * baseTransform, false);

  // And another in the lower-right, filled.
  drawCircle(focusArea(0.5, 0.5, 1.0, 1.0) * baseTransform, true);

  // Draw the round buttons.
  drawRoundButtons(focusArea(0.70, 0.25, 1.0, 0.55) * baseTransform);

  // Draw the dpad.
  drawDPadButtons(focusArea(0.0, 0.25, 0.3, 0.55) * baseTransform);

  // Draw the shoulder buttons.
  drawShoulderButtons(focusArea(0.0, 0.0, 0.3, 0.25) * baseTransform,
    true /*left*/);
  drawShoulderButtons(focusArea(0.7, 0.0, 1.0, 0.25) * baseTransform,
    false /*left*/);
}


void GVMainWindow::drawCircle(
  D2D1_MATRIX_3X2_F transform,
  bool fill)
{
  m_renderTarget->SetTransform(transform);

  D2D1_ELLIPSE circle;
  circle.point.x = 0.5;
  circle.point.y = 0.5;
  circle.radiusX = 0.4;
  circle.radiusY = 0.4;

  // Draw the outline always since the stroke width means the outer
  // edge is a bit larger than the filled ellipse.
  m_renderTarget->DrawEllipse(
    circle,
    m_linesBrush,
    3.0,                               // strokeWidth in pixels
    m_strokeStyleFixedThickness);      // strokeStyle

  if (fill) {
    m_renderTarget->FillEllipse(
      circle,
      m_linesBrush);
  }
}


void GVMainWindow::drawSquare(
  D2D1_MATRIX_3X2_F transform,
  bool fill)
{
  drawPartiallyFilledSquare(transform, fill? 1.0 : 0.0);
}


void GVMainWindow::drawPartiallyFilledSquare(
  D2D1_MATRIX_3X2_F transform,
  float fillAmount)
{
  m_renderTarget->SetTransform(transform);

  D2D1_RECT_F square;
  square.left = 0.1;
  square.top = 0.1;
  square.right = 0.9;
  square.bottom = 0.9;

  // Draw the outline always since the stroke width means the outer
  // edge is a bit larger than the filled ellipse.
  m_renderTarget->DrawRectangle(
    square,
    m_linesBrush,
    3.0,                               // strokeWidth in pixels
    m_strokeStyleFixedThickness);      // strokeStyle

  if (fillAmount > 0) {
    square.top = square.bottom - (square.bottom-square.top) * fillAmount;

    m_renderTarget->FillRectangle(
      square,
      m_linesBrush);
  }
}


void GVMainWindow::drawRoundButtons(
  D2D1_MATRIX_3X2_F transform)
{
  WORD buttons = m_controllerState.Gamepad.wButtons;

  // Button masks, starting at top, then going clockwise.
  WORD masks[4] = {
    XINPUT_GAMEPAD_Y,        // Top, PS triangle
    XINPUT_GAMEPAD_B,        // Right, PS circle
    XINPUT_GAMEPAD_A,        // Bottom, PS X
    XINPUT_GAMEPAD_X,        // Left, PS square
  };

  for (int i=0; i < 4; ++i) {
    drawCircle(focusArea(0.3, 0, 0.7, 0.4) * transform,
      buttons & masks[i]);

    // Rotate the transform 90 degrees around the center.
    transform =
      D2D1::Matrix3x2F::Rotation(90, D2D1::Point2F(0.5, 0.5)) * transform;
  }
}


void GVMainWindow::drawDPadButtons(
  D2D1_MATRIX_3X2_F transform)
{
  WORD buttons = m_controllerState.Gamepad.wButtons;

  // Button masks, starting at top, then going clockwise.
  WORD masks[4] = {
    XINPUT_GAMEPAD_DPAD_UP,
    XINPUT_GAMEPAD_DPAD_RIGHT,
    XINPUT_GAMEPAD_DPAD_DOWN,
    XINPUT_GAMEPAD_DPAD_LEFT,
  };

  for (int i=0; i < 4; ++i) {
    drawSquare(focusArea(0.3, 0, 0.7, 0.4) * transform,
      buttons & masks[i]);

    // Rotate the transform 90 degrees around the center.
    transform =
      D2D1::Matrix3x2F::Rotation(90, D2D1::Point2F(0.5, 0.5)) * transform;
  }
}


void GVMainWindow::drawShoulderButtons(
  D2D1_MATRIX_3X2_F transform,
  bool leftSide)
{
  WORD buttons = m_controllerState.Gamepad.wButtons;
  WORD mask = (leftSide? XINPUT_GAMEPAD_LEFT_SHOULDER :
                         XINPUT_GAMEPAD_RIGHT_SHOULDER);

  // Bumper.
  drawSquare(focusArea(0.0, 0.7, 1.0, 1.0) * transform,
    buttons & mask);

  BYTE trigger = (leftSide? m_controllerState.Gamepad.bLeftTrigger :
                            m_controllerState.Gamepad.bRightTrigger);
  float fillAmount = trigger / 255.0;

  // Trigger.
  drawPartiallyFilledSquare(focusArea(0.0, 0.0, 1.0, 0.7) * transform,
    fillAmount);
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

      // Create a timer for polling the controller.
      UINT_PTR id = SetTimer(m_hwnd, 1, 16 /*ms*/, nullptr /*proc*/);
      if (!id) {
        winapiDie(L"SetTimer");
      }
      assert(id == 1);

      createDeviceIndependentResources();
      return 0;
    }

    case WM_DESTROY:
      TRACE2(L"received WM_DESTROY");
      if (!KillTimer(m_hwnd, 1)) {
        winapiDie(L"KillTimer");
      }
      discardGraphicsResources();
      destroyDeviceIndependentResources();
      PostQuitMessage(0);
      return 0;

    case WM_TIMER:
      onTimer(wParam);
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
