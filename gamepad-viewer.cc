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
#include <cmath>                       // std::{cos, sin, atan2, sqrt}
#include <cstdlib>                     // std::{getenv, atoi}
#include <cstring>                     // std::memset
#include <iomanip>                     // std::{dec, hex}
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


// Menu item identifiers.
enum {
  IDM_SET_LINE_COLOR = 1,
  IDM_TOGGLE_TEXT = 2,
  IDM_TOGGLE_TOPMOST = 3,
  IDM_SMALLER_WINDOW = 4,
  IDM_LARGER_WINDOW = 5,
};


// Constants used to control the size, shape, and position of the
// controls in the UI.  I'm thinking to eventually put these into a
// configuration file; defining them here is a first step.
//
// All of these are in [0,1], representing fractional distances of the
// whole within either the whole UI or a parent button cluster.

// Distance from top to center of face button cluster and center of
// select/start cluster.
float const c_faceButtonsY = 0.42;

// Radius of face button clusters.
float const c_faceButtonsR = 0.15;

// Radius of one of the round face buttons.
float const c_roundButtonR = 0.20;

// Square radius of one of the dpad buttons.
float const c_dpadButtonR = 0.15;

// Distance from side to center of shoulder buttons.
float const c_shoulderButtonsX = 0.15;

// Radius of shoulder button cluster.
float const c_shoulderButtonsR = 0.125;

// Vertical radius of a bumper button within its shoulder cluster.
float const c_bumperVR = 0.15;

// Vertical radius of a trigger box within its shoulder cluster.
float const c_triggerVR = 0.35;

// Radius of each stick display cluster.
float const c_stickR = 0.25;

// Radius of the always-visible circle around the stick thumb.
float const c_stickOutlineR = 0.4;

// Maximum distance of the thumb from its center.
float const c_stickMaxDeflectR = 0.3;

// Radius of the filled circle representing the thumb.
float const c_stickThumbR = 0.1;

// Horizontal distance from the center line to the sel/start buttons.
float const c_selStartX = 0.08;

// Horizontal and vertical radii for sel/start.
float const c_selStartHR = 0.05;
float const c_selStartVR = 0.03;

// Distance from the top to the central circle.
float const c_centralCircleY = 0.52;

// Radius of the central circle.
float const c_centralCircleR = 0.035;


// Distance that `drawCircle` and `drawSquare` leave between the edge of
// the circle and the edge of its nominal area.
float const c_circleMargin = 0.1;

// Width in pixels of the lines.
float const c_lineWidthPixels = 3.0;


GVMainWindow::GVMainWindow()
  : m_d2dFactory(nullptr),
    m_writeFactory(nullptr),
    m_textFormat(nullptr),
    m_strokeStyleFixedThickness(nullptr),
    m_contextMenu(nullptr),
    m_linesColorref(RGB(128, 128, 224)),     // Pale blue.
    m_renderTarget(nullptr),
    m_textBrush(nullptr),
    m_linesBrush(nullptr),
    m_controllerState(),
    m_hasControllerState(false),
    m_lastDragPoint{},
    m_movingWindow(false),
    m_showText(false),
    m_topmostWindow(false)
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

  createContextMenu();
}


void GVMainWindow::destroyDeviceIndependentResources()
{
  safeRelease(m_d2dFactory);
  safeRelease(m_writeFactory);
  safeRelease(m_textFormat);
  safeRelease(m_strokeStyleFixedThickness);

  destroyContextMenu();
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

    createLinesBrushes();
  }
}


void GVMainWindow::destroyGraphicsResources()
{
  safeRelease(m_renderTarget);
  destroyLinesBrushes();
}


static float BYTE_to_float(BYTE b)
{
  return (float)b / 255.0f;
}


static D2D1_COLOR_F COLORREF_to_ColorF(COLORREF cr)
{
  return D2D1::ColorF(
    BYTE_to_float(GetRValue(cr)),
    BYTE_to_float(GetGValue(cr)),
    BYTE_to_float(GetBValue(cr)));
}


void GVMainWindow::createLinesBrushes()
{
  D2D1_COLOR_F linesColor = COLORREF_to_ColorF(m_linesColorref);
  CALL_HR_WINAPI(m_renderTarget->CreateSolidColorBrush,
    linesColor,
    &m_textBrush);
  assert(m_textBrush);

  CALL_HR_WINAPI(m_renderTarget->CreateSolidColorBrush,
    linesColor,
    &m_linesBrush);
}


void GVMainWindow::destroyLinesBrushes()
{
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

  // Reset the transform.
  m_renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

  // Draw the controller buttons, etc.
  drawControllerState();

  HRESULT hr = m_renderTarget->EndDraw();
  if (hr == HRESULT(D2DERR_RECREATE_TARGET)) {
    // This is a normal condition (but `FAILED(hr)` is still true) that
    // means the target device has become invalid.  Dispose of resources
    // and prepare to re-create them.
    TRACE2(L"createGraphicsResources: D2DERR_RECREATE_TARGET");
    destroyGraphicsResources();
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


// Create a transformation matrix centered on (x,y) with horizontal
// radius `hr` and vertical radius `vr`.
static D2D1_MATRIX_3X2_F focusPtHVR(
  float x,
  float y,
  float hr,
  float vr)
{
  return focusArea(x - hr, y - vr,
                   x + hr, y + vr);
}

// Create a transformation matrix centered on (x,y) with square radius
// `r`.
static D2D1_MATRIX_3X2_F focusPtR(
  float x,
  float y,
  float r)
{
  return focusPtHVR(x, y, r, r);
}


void GVMainWindow::drawControllerState()
{
  D2D1_SIZE_F renderTargetSize = m_renderTarget->GetSize();

  if (m_showText) {
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

    std::wstring s = oss.str();
    m_renderTarget->DrawText(
      s.data(),
      s.size(),
      m_textFormat,
      D2D1::RectF(150, 10, renderTargetSize.width, renderTargetSize.height),
      m_textBrush);
  }

  if (!( renderTargetSize.width > 0 && renderTargetSize.height > 0 )) {
    // Bail if the sizes are zero.
    return;
  }

  // Create a coordinate system where the upper-left is (0,0) and the
  // lower-right is (1,1).
  D2D1_MATRIX_3X2_F baseTransform =
    D2D1::Matrix3x2F::Scale(renderTargetSize.width, renderTargetSize.height);

  // Draw the round buttons.
  drawRoundButtons(
    focusPtR(1.0 - c_faceButtonsR, c_faceButtonsY, c_faceButtonsR) *
    baseTransform);

  // Draw the dpad.
  drawDPadButtons(
    focusPtR(c_faceButtonsR, c_faceButtonsY, c_faceButtonsR) *
    baseTransform);

  // Draw the shoulder buttons.
  drawShoulderButtons(
    focusPtR(c_shoulderButtonsX, c_shoulderButtonsR, c_shoulderButtonsR) * baseTransform,
    true /*left*/);
  drawShoulderButtons(
    focusPtR(1.0 - c_shoulderButtonsX, c_shoulderButtonsR, c_shoulderButtonsR) * baseTransform,
    false /*left*/);

  // Draw the sticks.
  drawStick(
    focusPtR(c_stickR, 1.0 - c_stickR, c_stickR) * baseTransform,
    true /*left*/);
  drawStick(
    focusPtR(1.0 - c_stickR, 1.0 - c_stickR, c_stickR) * baseTransform,
    false /*left*/);

  // Draw the select and start buttons.
  drawSelStartButton(
    focusPtHVR(0.5 - c_selStartX, c_faceButtonsY,
               c_selStartHR,      c_selStartVR)   * baseTransform,
    true /*left*/);
  drawSelStartButton(
    focusPtHVR(0.5 + c_selStartX, c_faceButtonsY,
               c_selStartHR,      c_selStartVR)   * baseTransform,
    false /*left*/);

  // Draw a central circle that could be considered to mimic the
  // Playstation button, but in this app mostly functions as a larger
  // place for the mouse to be clicked since the rest of the UI consists
  // of thin lines that are hard to click.
  drawCentralCircle(
    focusPtR(0.5, c_centralCircleY, c_centralCircleR) * baseTransform);
}


void GVMainWindow::drawCircle(
  D2D1_MATRIX_3X2_F transform,
  bool fill)
{
  m_renderTarget->SetTransform(transform);

  D2D1_ELLIPSE circle;
  circle.point.x = 0.5;
  circle.point.y = 0.5;
  circle.radiusX = 0.5 - c_circleMargin;
  circle.radiusY = 0.5 - c_circleMargin;

  // Draw the outline always since the stroke width means the outer
  // edge is a bit larger than the filled ellipse.
  m_renderTarget->DrawEllipse(
    circle,
    m_linesBrush,
    c_lineWidthPixels,                 // strokeWidth in pixels
    m_strokeStyleFixedThickness);      // strokeStyle

  if (fill) {
    m_renderTarget->FillEllipse(
      circle,
      m_linesBrush);
  }
}


void GVMainWindow::drawCircleAt(
  D2D1_MATRIX_3X2_F transform,
  float x,
  float y,
  float r,
  bool fill)
{
  drawCircle(focusArea(x-r, y-r, x+r, y+r) * transform, fill);
}


void GVMainWindow::drawSquare(
  D2D1_MATRIX_3X2_F transform,
  bool fill)
{
  drawPartiallyFilledSquare(transform, fill? 1.0 : 0.0, 1.0 /*fillHR*/);
}


void GVMainWindow::drawPartiallyFilledSquare(
  D2D1_MATRIX_3X2_F transform,
  float fillAmount,
  float fillHR)
{
  m_renderTarget->SetTransform(transform);

  D2D1_RECT_F square;
  square.left = c_circleMargin;
  square.top = c_circleMargin;
  square.right = 1.0 - c_circleMargin;
  square.bottom = 1.0 - c_circleMargin;

  // Draw the outline always since the stroke width means the outer
  // edge is a bit larger than the filled ellipse.
  m_renderTarget->DrawRectangle(
    square,
    m_linesBrush,
    c_lineWidthPixels,                 // strokeWidth
    m_strokeStyleFixedThickness);      // strokeStyle

  if (fillAmount > 0) {
    square.top = square.bottom - (square.bottom-square.top) * fillAmount;

    // Apply `fillHR` to `square`.
    float hr = (square.right - square.left) / 2.0;
    float x = (square.right + square.left) / 2.0;
    square.left = x - hr * fillHR;
    square.right = x + hr * fillHR;

    m_renderTarget->FillRectangle(
      square,
      m_linesBrush);
  }
}


void GVMainWindow::drawLine(
  D2D1_MATRIX_3X2_F transform,
  float x1,
  float y1,
  float x2,
  float y2)
{
  m_renderTarget->SetTransform(transform);

  m_renderTarget->DrawLine(
    D2D1::Point2F(x1, y1),
    D2D1::Point2F(x2, y2),
    m_linesBrush,
    c_lineWidthPixels,                 // strokeWidth,
    m_strokeStyleFixedThickness);      // strokeStyle
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
    drawCircle(focusPtR(0.5, c_roundButtonR, c_roundButtonR) * transform,
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
    drawSquare(focusPtR(0.5, c_dpadButtonR, c_dpadButtonR) * transform,
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
  drawSquare(
    focusPtHVR(0.5, 1.0 - c_bumperVR, 0.5, c_bumperVR) * transform,
    buttons & mask);

  BYTE trigger = (leftSide? m_controllerState.Gamepad.bLeftTrigger :
                            m_controllerState.Gamepad.bRightTrigger);
  float fillAmount = trigger / 255.0;

  // Trigger.
  //
  // If `trigger` exceeds the dead zone threshold, then the fill is the
  // entire rectangle width.  But if not, it is only half of the width
  // in order to indicate that the game may not regiser it.
  //
  drawPartiallyFilledSquare(
    focusPtHVR(0.5, c_triggerVR, 0.5, c_triggerVR) * transform,
    fillAmount,
    trigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD? 1.0 : 0.5);
}


void GVMainWindow::drawStick(
  D2D1_MATRIX_3X2_F transform,
  bool leftSide)
{
  // Outline.
  drawCircleAt(transform, 0.5, 0.5, c_stickOutlineR, false /*fill*/);

  // Raw stick position in [-32768,32767], positive being rightward.
  float rawX = (leftSide? m_controllerState.Gamepad.sThumbLX :
                          m_controllerState.Gamepad.sThumbRX);

  // Raw stick position in [-32768,32767], positive being upward.
  float rawY = (leftSide? m_controllerState.Gamepad.sThumbLY :
                          m_controllerState.Gamepad.sThumbRY);

  // Dead zone value that MS recommends.
  float deadZone = (leftSide? XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE :
                              XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

  // Magnitude of deflection in the raw units.
  float magnitude = std::sqrt(rawX*rawX + rawY*rawY);

  if (magnitude > deadZone) {
    // Truncate anything outside the circle.
    if (magnitude > 32767) {
      magnitude = 32767;
    }

    // Remove the dead zone contribution.
    magnitude -= deadZone;

    // Scale what remains to [0,1].
    magnitude = magnitude / (32767 - deadZone);

    // Deflection angle.  Flip the Y coordinate here to account for the
    // raw units having the oppositely oriented vertical axis.
    float angleRadians = std::atan2(-rawY, rawX);

    // Deflection distances in [-1,1].
    float deflectX = magnitude * std::cos(angleRadians);
    float deflectY = magnitude * std::sin(angleRadians);

    // Filled circle representing the grippy part.
    float spotX = 0.5 + deflectX * c_stickMaxDeflectR;
    float spotY = 0.5 + deflectY * c_stickMaxDeflectR;
    drawCircleAt(transform, spotX, spotY, c_stickThumbR, true /*fill*/);

    // Line from center to circle showing the deflection angle, even
    // when the thumb is close to the center.
    float edgeX = 0.5 + std::cos(angleRadians) * c_stickMaxDeflectR;
    float edgeY = 0.5 + std::sin(angleRadians) * c_stickMaxDeflectR;
    drawLine(transform, 0.5, 0.5, edgeX, edgeY);
  }

  WORD buttons = m_controllerState.Gamepad.wButtons;
  WORD mask = (leftSide? XINPUT_GAMEPAD_LEFT_THUMB :
                         XINPUT_GAMEPAD_RIGHT_THUMB);

  // Stick click button.
  if (buttons & mask) {
    drawCircle(transform, false /*fill*/);
  }
}


void GVMainWindow::drawSelStartButton(
  D2D1_MATRIX_3X2_F transform,
  bool leftSide)
{
  WORD buttons = m_controllerState.Gamepad.wButtons;
  WORD mask = (leftSide? XINPUT_GAMEPAD_BACK :  // PS select
                         XINPUT_GAMEPAD_START);

  drawSquare(transform, buttons & mask);
}


void GVMainWindow::drawCentralCircle(
  D2D1_MATRIX_3X2_F transform)
{
  drawCircle(transform, true /*fill*/);
}


void GVMainWindow::onResize()
{
  if (m_renderTarget) {
    m_renderTarget->Resize(getClientRectSizeU());

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
  TRACE2(L"onKeyDown:" << std::hex <<
         TRVAL(wParam) << TRVAL(lParam) << std::dec);

  switch (wParam) {
    case 'Q':
      // Q to quit.
      TRACE2(L"Saw Q keypress.");
      PostMessage(m_hwnd, WM_CLOSE, 0, 0);
      return true;

    case 'C':
      runColorChooser();
      return true;

    case 'S':
      toggleShowText();
      return true;

    case 'T':
      toggleTopmost();
      return true;

    case VK_OEM_MINUS:
      resizeWindow(-50);
      return true;

    case VK_OEM_PLUS:
      resizeWindow(+50);
      return true;
  }

  // Not handled.
  return false;
}


void GVMainWindow::resizeWindow(int delta)
{
  RECT r;
  GetWindowRect(m_hwnd, &r);

  int w = std::max(r.right - r.left + delta, 50L);
  int h = std::max(r.bottom - r.top + delta, 50L);

  TRACE2(L"resizeWindow:" << TRVAL(w) << TRVAL(h));

  MoveWindow(m_hwnd,
    r.left,
    r.top,
    w,
    h,
    false /*repaint*/);
}


void GVMainWindow::createContextMenu()
{
  m_contextMenu = CreatePopupMenu();
  if (!m_contextMenu) {
    winapiDie(L"CreatePopupMenu");
  }

  appendContextMenu(IDM_SET_LINE_COLOR, L"Set line color (C)");
  appendContextMenu(IDM_TOGGLE_TEXT,    L"Toggle text display (S)");
  appendContextMenu(IDM_TOGGLE_TOPMOST, L"Toggle topmost (T)");
  appendContextMenu(IDM_SMALLER_WINDOW, L"Make display smaller (-)");
  appendContextMenu(IDM_LARGER_WINDOW,  L"Make display larger (+)");
}


void GVMainWindow::appendContextMenu(int id, wchar_t const *label)
{
  if (!AppendMenu(
         m_contextMenu,
         MF_STRING,
         id,
         label)) {
    winapiDie(L"AppendMenu");
  }
}


void GVMainWindow::destroyContextMenu()
{
  if (!DestroyMenu(m_contextMenu)) {
    winapiDie(L"DestroyMenu");
  }
  m_contextMenu = nullptr;
}


void GVMainWindow::onContextMenu(int x, int y)
{
  POINT pt = {x,y};
  ClientToScreen(m_hwnd, &pt);

  TRACE2(L"onContextMenu:" << TRVAL(x) << TRVAL(y));

  if (!TrackPopupMenu(
         m_contextMenu,
         TPM_LEFTALIGN | TPM_TOPALIGN,
         x,
         y,
         0,                  // nReserved
         m_hwnd,
         nullptr)) {         // prcRect
    // At least sometimes this triggers a second time with the error
    // "Popup menu already active.", so ignore it.
    //winapiDie(L"TrackPopupMenu");
  }
}


bool GVMainWindow::onCommand(WPARAM wParam, LPARAM lParam)
{
  TRACE2(L"onCommand:" << std::hex <<
         TRVAL(wParam) << TRVAL(lParam) << std::dec);

  switch (wParam) {
    case IDM_SET_LINE_COLOR:
      runColorChooser();
      return true;

    case IDM_TOGGLE_TEXT:
      toggleShowText();
      return true;

    case IDM_TOGGLE_TOPMOST:
      toggleTopmost();
      return true;

    case IDM_SMALLER_WINDOW:
      resizeWindow(-50);
      return true;

    case IDM_LARGER_WINDOW:
      resizeWindow(+50);
      return true;
  }

  return false;
}


void GVMainWindow::runColorChooser()
{
  TRACE2(L"runColorChooser");
  CHOOSECOLOR cc;
  ZeroMemory(&cc, sizeof(cc));
  cc.lStructSize = sizeof(cc);
  cc.hwndOwner = m_hwnd;
  cc.rgbResult = m_linesColorref;
  cc.Flags = CC_RGBINIT | CC_FULLOPEN;

  // This is required even if we say CC_PREVENTFULLOPEN.
  static COLORREF customColors[16];
  cc.lpCustColors = (LPDWORD)customColors;

  if (ChooseColor(&cc)) {
    m_linesColorref = cc.rgbResult;
    int r = GetRValue(m_linesColorref);
    int g = GetGValue(m_linesColorref);
    int b = GetBValue(m_linesColorref);
    TRACE2(L"Got color:" <<
      TRVAL(r) <<
      TRVAL(g) <<
      TRVAL(b));

    destroyLinesBrushes();
    createLinesBrushes();
    invalidateAllPixels();
  }
}


void GVMainWindow::toggleShowText()
{
  m_showText = !m_showText;
  invalidateAllPixels();
}


void GVMainWindow::toggleTopmost()
{
  m_topmostWindow = !m_topmostWindow;
  TRACE2(L"toggleTopmost: now " << m_topmostWindow);

  if (!SetWindowPos(
         m_hwnd,
         m_topmostWindow? HWND_TOPMOST : HWND_NOTOPMOST,
         0,0,0,0,                      // New pos/size, ignored.
         SWP_NOMOVE | SWP_NOSIZE)) {   // Ignore pos/size.
    winapiDie(L"SetWindowPos");
  }
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
      destroyGraphicsResources();
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

    case WM_KEYDOWN:
      if (onKeyDown(wParam, lParam)) {
        // Handled.
        return 0;
      }
      break;

    case WM_LBUTTONDOWN:
      m_movingWindow = true;

      // Capture the mouse during the drag so even if the mouse moves
      // outside the opaque area we keep tracking it.
      SetCapture(m_hwnd);

      GetCursorPos(&m_lastDragPoint);
      return 0;

    case WM_LBUTTONUP:
      ReleaseCapture();
      m_movingWindow = false;
      return 0;

    case WM_MOUSEMOVE:
      if (m_movingWindow) {
        POINT pt;
        GetCursorPos(&pt);

        RECT r;
        GetWindowRect(m_hwnd, &r);

        // Prepare to adjust the window rectangle by the amount the
        // mouse moved.
        int dx = pt.x - m_lastDragPoint.x;
        int dy = pt.y - m_lastDragPoint.y;

        // Save the most recent drag point.
        m_lastDragPoint = pt;

        // Move the window.
        //
        // This does not work as well as it could because when the
        // mouse moves enough to get outside the thin lines, I stop
        // getting mouse move messages.
        //
        MoveWindow(m_hwnd,
          r.left + dx,
          r.top + dy,
          r.right - r.left,
          r.bottom - r.top,
          false /*repaint*/);

        return 0;
      }
      break;

    case WM_CONTEXTMENU:
      onContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      return 0;

    case WM_COMMAND:
      if (onCommand(wParam, lParam)) {
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
