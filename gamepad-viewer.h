// gamepad-viewer.h
// `GVMainWindow` class, the main window class of the gamepad viewer.

#ifndef GAMEPAD_VIEWER_H
#define GAMEPAD_VIEWER_H

#include "base-window.h"               // BaseWindow

#include <d2d1.h>                      // Direct2D
#include <windows.h>                   // Windows API


// Main window of the gamepad viewer.
class GVMainWindow : public BaseWindow {
public:      // data
  // D2D factory used to create the render target.  This is, I think,
  // the root interface to D2D.
  ID2D1Factory *m_d2dFactory;

  // D2D render target associated with the main window.
  ID2D1HwndRenderTarget *m_renderTarget;

  // A solid yellow brush associated with `m_renderTarget` and used to
  // fill the ellipse.
  ID2D1SolidColorBrush *m_brush;

  // Shape of ellipse to draw.
  D2D1_ELLIPSE m_ellipse;

  // Clockwise rotation to apply to the drawn ellipse, in degrees.
  int m_rotDegrees;

public:      // methods
  GVMainWindow()
    : m_d2dFactory(nullptr),
      m_renderTarget(nullptr),
      m_brush(nullptr),
      m_ellipse(),
      m_rotDegrees(0)
  {}

  // Return the client rectangle size as a D2D1_SIZE_U.
  D2D1_SIZE_U getClientRectSizeU() const;

  // Set `m_ellipse` based on `m_renderTarget`.
  void calculateLayout();

  // If necessary, populate `m_renderTarget`, `m_brush`, and
  // `m_ellipse`.
  void createGraphicsResources();

  // Release and nullify `m_renderTarget` and `m_brush`.
  void discardGraphicsResources();

  // Handle `WM_PAINT`.
  void onPaint();

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
