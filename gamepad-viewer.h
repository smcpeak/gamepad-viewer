// gamepad-viewer.h
// `GVMainWindow` class, the main window class of the gamepad viewer.

// See license.txt for copyright and terms of use.

#ifndef GAMEPAD_VIEWER_H
#define GAMEPAD_VIEWER_H

#include "base-window.h"               // BaseWindow
#include "button-timer.h"              // ButtonTimer
#include "controller-state.h"          // ControllerState
#include "gpv-config.h"                // GPVConfig

#include <d2d1.h>                      // Direct2D
#include <d2d1_1.h>                    // ID2D1StrokeStyle1, ID2DFactory1
#include <dwrite.h>                    // IDWriteFactory, IDWriteTextFormat
#include <windows.h>                   // Windows API
#include <xinput.h>                    // XINPUT_STATE


// A UI element role that corresponds to a color.
enum GVColorRole {
  // No color; used to indicate, e.g., an unfilled interior.
  GVCR_NONE,

  // The normal color used for most lines.
  GVCR_NORMAL,

  // The highlight color for chevrons.
  GVCR_HIGHLIGHT,

  // Color to indicate parry is active.
  GVCR_PARRY_ACTIVE,

  // Color to indicate parry is inactive.
  GVCR_PARRY_INACTIVE,

  // Background color behind the text that shows the milliseconds on
  // the parry timer.
  GVCR_TEXT_BACKGROUND,

  // Text background colors for dodge invulnerability timer, depending
  // on whether it is active (invulnerable).
  GVCR_DODGE_ACTIVE,
  GVCR_DODGE_INACTIVE,

  NUM_GV_COLOR_ROLES
};


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

  // The sub-menu listing the controller IDs.
  HMENU m_controllerIDMenu;

  // ----------------- D2D device-dependent resources ------------------
  // D2D render target associated with the main window.
  ID2D1HwndRenderTarget *m_renderTarget;

  // Brush for drawing text.
  ID2D1SolidColorBrush *m_textBrush;

  // Brush for drawing the thin lines that are always shown for buttons,
  // etc.
  ID2D1SolidColorBrush *m_linesBrush;

  // Brush for drawing highlight lines.
  ID2D1SolidColorBrush *m_highlightBrush;

  // Brushes for filling the parry timer when the parry effect is active
  // or inactive.
  ID2D1SolidColorBrush *m_parryActiveBrush;
  ID2D1SolidColorBrush *m_parryInactiveBrush;

  // Painted behind the parry timer text.
  ID2D1SolidColorBrush *m_textBackgroundBrush;

  // Painted behind the dodge timer text.
  ID2D1SolidColorBrush *m_dodgeActiveBrush;
  ID2D1SolidColorBrush *m_dodgeInactiveBrush;

  // ------------------------- Other app state -------------------------
  // User-adjustable configuration.
  GPVConfig m_config;

  // Current controller input.
  ControllerState m_controllerState;

  // Controller input state during the previous polling cycle.
  ControllerState m_prevControllerState;

  // Timer associated with pressing the parry button (L2).
  ButtonTimer m_parryTimer;

  // Timer associated with releasing the dodge button (XBox B,
  // PlayStation circle).
  ButtonTimer m_dodgeReleaseTimer;

  // Timer that tracks the invulnerability window associated with
  // dodging.  This starts at the same time as the release timer, but
  // then tracks both the invulnerability window and the recovery
  // window, and also handles dodge queueing.
  ButtonTimer m_dodgeInvulnerabilityTimer;

  // Last point where the mouse was seen pressed.
  POINT m_lastDragPoint;

  // If true, then we are moving the window by mouse dragging.
  bool m_movingWindow;

  // The controller ID the last time we drew the controller state.
  int m_lastShownControllerID;

public:      // methods
  GVMainWindow();

  // Current layout parameters.
  LayoutParams const &lp() const
    { return m_config.m_layoutParams; }

  // Create the device-independent resources.
  void createDeviceIndependentResources();

  // Destroy the device-independent resources.
  void destroyDeviceIndependentResources();

  // Set `m_controllerState` by polling the controller.
  void pollControllerState();

  // Is any button timer currently running?
  bool isAnyButtonTimerRunning() const;

  // Current state of buttons, etc.
  XINPUT_STATE const &inputState() const;

  // If the dodge invulnerability timer is active, return the number of
  // milliseconds since its timer started.  Otherwise return 0.
  DWORD dodgeInvulnerabilityTimerElapsedMS() const;

  // Is the invulnerability effect active according to the timer and
  // config?
  //
  // This is our best guess, based only on dodge button release events,
  // whether the player should be invulnerable right now.  It can be
  // wrong for many reasons, but should be more convenient, when
  // reviewing recordings, than manually counting frames.
  //
  bool isDodgeInvulnerabilityActive() const;

  // If the parry timer is active, return the number of milliseconds
  // since its.  Otherwise return 0.
  DWORD parryTimerElapsedMS() const;

  // Is the parry effect active according to the timer and config?
  bool isParryActive() const;

  // Evaulate the current dodge timer value and classify it as being a
  // certain number of frames before, after, or during the
  // invulnerability window, returning that classification as a string.
  // Also set `active` to true if invulnerability is active, and false
  // otherwise.
  //
  // This is meant to be meaningful when reviewing a recording and
  // examining the frame on which damage was taken (or would have been).
  //
  std::wstring dodgeAccuracyString(bool &active /*OUT*/) const;

  // Evaluate the current parry timer value as a parry accuracy
  // assessment, under the assumption that the frame we are showing is
  // the frame where either damage was received (for a failed parry) or
  // the game registered a successful parry.
  std::wstring parryAccuracyString() const;

  // Return the client rectangle size as a D2D1_SIZE_U.
  D2D1_SIZE_U getClientRectSizeU() const;

  // If necessary, create the device-independent resources.
  void createGraphicsResources();

  // Release and nullify the device-independent resources.
  void destroyGraphicsResources();

  // Create a single brush from `colorref`, storing its pointer in
  // `brush`.
  void createBrush(
    ID2D1SolidColorBrush *&brush, COLORREF colorref);

  // Create/destroy the brushes used for drawing lines and fills.
  void createLinesBrushes();
  void destroyLinesBrushes();

  // Return the brush to use for a `color`.
  ID2D1SolidColorBrush *brushForColorRole(GVColorRole color) const;

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
  void drawSquare(
    D2D1_MATRIX_3X2_F transform,
    GVColorRole color,
    float margin,
    bool fill);

  // Draw a square that is filled, from the bottom, by `fillAmount`.
  // `fillHR` is the horizontal radius of the filled portion, where 1.0
  // represents filling the box completely.
  //
  // The square is drawn `margin` proportional units inside the edges of
  // `transform`.
  void drawPartiallyFilledSquare(
    D2D1_MATRIX_3X2_F transform,
    GVColorRole color,
    float margin,
    float fillAmount,
    float fillHR);

  // Draw a line from (x1,y1) to (x2,y2).
  void drawLine(
    D2D1_MATRIX_3X2_F transform,
    float x1,
    float y1,
    float x2,
    float y2,
    GVColorRole color);

  // Draw `str` with its upper-left corner at `textCursor`, painting the
  // actually used region with `bgColorRole` first.
  void drawTextWithBackground(
    std::wstring const &str,
    D2D1_POINT_2F const &textCursor,
    GVColorRole bgColorRole);

  // Draw the round face buttons.
  void drawRoundButtons(D2D1_MATRIX_3X2_F transform);

  // Draw the dpad buttons.
  void drawDPadButtons(D2D1_MATRIX_3X2_F transform);

  // Draw the left or right shoulder button and trigger.
  void drawShoulderButtons(D2D1_MATRIX_3X2_F transform, bool leftSide);

  // Draw the parry timer.
  void drawParryTimer(D2D1_MATRIX_3X2_F transform);

  // Draw one of the sticks.
  void drawStick(D2D1_MATRIX_3X2_F transform, bool leftSide);

  // Draw the speed indicator on the left thumb.
  void drawSpeedIndicator(
    D2D1_MATRIX_3X2_F transform,
    float spotX,
    float spotY,
    float angleRadians,
    int speed);

  // Draw a up-pointing chevron in the nominal box.  Offset its Y
  // coordinate by `dy`.
  void drawChevron(D2D1_MATRIX_3X2_F transform, float dy);

  // Draw one of the select/start buttons.
  void drawSelStartButton(D2D1_MATRIX_3X2_F transform, bool leftSide);

  // Draw the central filled circle.
  void drawCentralCircle(D2D1_MATRIX_3X2_F transform);

  // Cause a repaint event that will redraw the entire window.
  void invalidateAllPixels();

  // Handle `WM_SIZE` or `WM_WINDOWPOSCHANGED` with a new size.
  void onResize();

  // Handle `WM_KEYDOWN`.  Return true if handled.
  bool onKeyDown(WPARAM wParam, LPARAM lParam);

  // Resize the window by `delta` pixels in both directions.
  void resizeWindow(int delta);

  // Create/destroy `m_contextMenu`.
  void createContextMenu();
  void appendContextMenu(int id, wchar_t const *label);
  void appendMenu(HMENU menu, int id, wchar_t const *label);
  void destroyContextMenu();

  // Handle `WM_CONTEXTMENU`.  The mouse was clicked at (x,y) in client
  // coordinates.
  void onContextMenu(int x, int y);

  // Handle `WM_COMMAND`.  Return true if handled.
  bool onCommand(WPARAM wParam, LPARAM lParam);

  // Show the dialog that lets the user pick the lines color.  If
  // `highlight`, we are selecting the highlight color, otherwise the
  // normal lines color.
  void runColorChooser(bool highlight);

  // Toggle whether we show the text.
  void toggleShowText();

  // Toggle whether this window is topmost.
  void toggleTopmost();

  // Set as topmost or not depending on `tm`.
  void setTopmost(bool tm);

  // Toggle whether we are showing the parry accuracy.
  void toggleShowParryAccuracyText();

  // Toggle whether we are showing the parry timer as text.
  void toggleShowParryTimeText();

  // Toggle whether to show the dodge invulnerability timer.
  void toggleShowDodgeInvulnerabilityTimer();

  // Return the name of the file in which configuration information is
  // stored.
  std::string getConfigFilename() const;

  // Attempt to read the configuration from the file.  If the file does
  // not exist, skip it.  If it does, but there is an error, print a
  // tracing message but continue.
  void loadConfiguration();

  // Attempt to write the current configuration to the file.  On error,
  // print a tracing message but keep going.
  void saveConfiguration() const;

  // Handle `WM_WINDOWPOSCHANGED`.
  void onWindowPosChanged(WINDOWPOS const *wp);

  // Minimize the game viewer window.
  void minimizeWindow();

  // BaseWindow methods.
  virtual LRESULT handleMessage(
    UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};


#endif // GAMEPAD_VIEWER_H
