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

  // "Text format" object used by `DrawText`.
  IDWriteTextFormat *m_textFormat;

  // Stroke style to avoid transforming its thickness.
  ID2D1StrokeStyle1 *m_strokeStyleFixedThickness;

  // The menu to show on right-click.
  HMENU m_contextMenu;

  // Color to use to draw the lines.
  COLORREF m_linesColorref;

  // ----------------- D2D device-dependent resources ------------------
  // D2D render target associated with the main window.
  ID2D1HwndRenderTarget *m_renderTarget;

  // Brush for drawing text.
  ID2D1SolidColorBrush *m_textBrush;

  // Brush for drawing the thin lines that are always shown for buttons,
  // etc.
  ID2D1SolidColorBrush *m_linesBrush;

  // ------------------------- Other app state -------------------------
  // Current controller input.
  XINPUT_STATE m_controllerState;

  // True if the last poll attempt succeeded.
  bool m_hasControllerState;

  // Last point where the mouse was seen pressed.
  POINT m_lastDragPoint;

  // If true, then we are moving the window by mouse dragging.
  bool m_movingWindow;

  // If true, show the textual display of the controller inputs.
  bool m_showText;

  // If true, set our window to be on top of all others (that are not
  // also topmost).
  bool m_topmostWindow;

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

  // If necessary, create the device-independent resources.
  void createGraphicsResources();

  // Release and nullify the device-independent resources.
  void destroyGraphicsResources();

  // Create/destroy `m_textBrush` and `m_linesBrush`.
  void createLinesBrushes();
  void destroyLinesBrushes();

  // Handle `WM_TIMER`.
  void onTimer(WPARAM wParam);

  // Handle `WM_PAINT`.
  void onPaint();

  // Draw the controller state on `m_renderTarget`.
  void drawControllerState();

  // Draw a centered circle mostly filling the box.
  void drawCircle(D2D1_MATRIX_3X2_F transform, bool fill);

  // Draw a circle centered at (x,y) with radius (r).
  void drawCircleAt(
    D2D1_MATRIX_3X2_F transform,
    float x,
    float y,
    float r,
    bool fill);

  // Draw a square in the box.
  void drawSquare(D2D1_MATRIX_3X2_F transform, bool fill);

  // Draw a square that is filled, from the bottom, by `fillAmount`.
  void drawPartiallyFilledSquare(
    D2D1_MATRIX_3X2_F transform,
    float fillAmount);

  // Draw a line from (x1,y1) to (x2,y2).
  void drawLine(
    D2D1_MATRIX_3X2_F transform,
    float x1,
    float y1,
    float x2,
    float y2);

  // Draw the round face buttons.
  void drawRoundButtons(D2D1_MATRIX_3X2_F transform);

  // Draw the dpad buttons.
  void drawDPadButtons(D2D1_MATRIX_3X2_F transform);

  // Draw the left or right shoulder button and trigger.
  void drawShoulderButtons(D2D1_MATRIX_3X2_F transform, bool leftSide);

  // Draw one of the sticks.
  void drawStick(D2D1_MATRIX_3X2_F transform, bool leftSide);

  // Draw one of the select/start buttons.
  void drawSelStartButton(D2D1_MATRIX_3X2_F transform, bool leftSide);

  // Draw the central filled circle.
  void drawCentralCircle(D2D1_MATRIX_3X2_F transform);

  // Cause a repaint event that will redraw the entire window.
  void invalidateAllPixels();

  // Handle `WM_SIZE`.
  void onResize();

  // Handle `WM_KEYDOWN`.  Return true if handled.
  bool onKeyDown(WPARAM wParam, LPARAM lParam);

  // Resize the window by `delta` pixels in both directions.
  void resizeWindow(int delta);

  // Create/destroy `m_contextMenu`.
  void createContextMenu();
  void appendContextMenu(int id, wchar_t const *label);
  void destroyContextMenu();

  // Handle `WM_CONTEXTMENU`.  The mouse was clicked at (x,y) in client
  // coordinates.
  void onContextMenu(int x, int y);

  // Handle `WM_COMMAND`.  Return true if handled.
  bool onCommand(WPARAM wParam, LPARAM lParam);

  // Show the dialog that lets the user pick the lines color.
  void runColorChooser();

  // Toggle whether we show the text.
  void toggleShowText();

  // Toggle whether this window is topmost.
  void toggleTopmost();

  // BaseWindow methods.
  virtual LRESULT handleMessage(
    UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};


#endif // GAMEPAD_VIEWER_H
