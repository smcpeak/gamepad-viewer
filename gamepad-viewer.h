// gamepad-viewer.h
// `GVMainWindow` class, the main window class of the gamepad viewer.

#ifndef GAMEPAD_VIEWER_H
#define GAMEPAD_VIEWER_H

#include "base-window.h"               // BaseWindow

#include <d2d1.h>                      // Direct2D
#include <d2d1_1.h>                    // ID2D1StrokeStyle1, ID2DFactory1
#include <dwrite.h>                    // IDWriteFactory, IDWriteTextFormat
#include <windows.h>                   // Windows API
#include <xinput.h>                    // XINPUT_STATE


// Main window of the gamepad viewer.
class GVMainWindow : public BaseWindow {
public:      // data
  // ---------------- D2D device-independent resources -----------------
  // D2D factory used to create the render target.  This is, I think,
  // the root interface to D2D.
  ID2D1Factory1 *m_d2dFactory;

  // DirectWrite factory, used to create `m_textFormat`.
  IDWriteFactory *m_writeFactory;

  // "Text format" object for ... what?
  IDWriteTextFormat *m_textFormat;

  // Stroke style to avoid transforming its thickness.
  ID2D1StrokeStyle1 *m_strokeStyleFixedThickness;

  // ----------------- D2D device-dependent resources ------------------
  // D2D render target associated with the main window.
  ID2D1HwndRenderTarget *m_renderTarget;

  // A solid yellow brush associated with `m_renderTarget` and used to
  // fill the ellipse.
  ID2D1SolidColorBrush *m_yellowBrush;

  // Brush for drawing text.
  ID2D1SolidColorBrush *m_textBrush;

  // Brush for drawing the thin lines that are always shown for buttons,
  // etc.
  ID2D1SolidColorBrush *m_linesBrush;

  // ------------------------- Other app state -------------------------
  // Shape of ellipse to draw.
  D2D1_ELLIPSE m_ellipse;

  // Clockwise rotation to apply to the drawn ellipse, in degrees.
  int m_rotDegrees;

  // Current controller input.
  XINPUT_STATE m_controllerState;

  // True if the last poll attempt succeeded.
  bool m_hasControllerState;

public:      // methods
  GVMainWindow();

  // Create the device-independent resources.
  void createDeviceIndependentResources();

  // Destroy the device-independent resources.
  void destroyDeviceIndependentResources();

  // Set `m_controllerState` by polling the controller.
  void pollControllerState();

  // Return the client rectangle size as a D2D1_SIZE_U.
  D2D1_SIZE_U getClientRectSizeU() const;

  // Set `m_ellipse` based on `m_renderTarget`.
  void calculateLayout();

  // If necessary, populate `m_renderTarget`, `m_brush`, and
  // `m_ellipse`.
  void createGraphicsResources();

  // Release and nullify `m_renderTarget` and `m_brush`.
  void discardGraphicsResources();

  // Handle `WM_TIMER`.
  void onTimer(WPARAM wParam);

  // Handle `WM_PAINT`.
  void onPaint();

  // Draw the controller state on `m_renderTarget`.
  void drawControllerState();

  // Draw a centered circle mostly filling the box.
  void drawCircle(D2D1_MATRIX_3X2_F transform, bool fill);

  // Draw a square in the box.
  void drawSquare(D2D1_MATRIX_3X2_F transform, bool fill);

  // Draw a square that is filled, from the bottom, by `fillAmount`.
  void drawPartiallyFilledSquare(
    D2D1_MATRIX_3X2_F transform,
    float fillAmount);

  // Draw the round face buttons.
  void drawRoundButtons(D2D1_MATRIX_3X2_F transform);

  // Draw the dpad buttons.
  void drawDPadButtons(D2D1_MATRIX_3X2_F transform);

  // Draw the left or right shoulder button and trigger.
  void drawShoulderButtons(D2D1_MATRIX_3X2_F transform, bool leftSide);

  // Cause a repaint event that will redraw the entire window.
  void invalidateAllPixels();

  // Handle `WM_SIZE`.
  void onResize();

  // Handle `WM_KEYDOWN`.  Return true if handled.
  bool onKeyDown(WPARAM wParam, LPARAM lParam);

  // BaseWindow methods.
  virtual LRESULT handleMessage(
    UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};


#endif // GAMEPAD_VIEWER_H
