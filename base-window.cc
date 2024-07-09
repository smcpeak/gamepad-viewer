// base-window.cc
// Code for `base-window` module.

#include "base-window.h"               // this module

#include "winapi-util.h"               // winapiDie, CreateWindowExWArgs

#include <cassert>                     // assert


wchar_t const *BaseWindow::s_windowClassName = L"Base Window Class";

bool BaseWindow::s_windowClassRegistered = false;


BaseWindow::~BaseWindow()
{}


BaseWindow::BaseWindow()
  : m_hwnd(nullptr)
{}


void BaseWindow::registerWindowClassIfNecessary()
{
  if (s_windowClassRegistered) {
    return;
  }

  WNDCLASS wc = { };

  wc.lpfnWndProc   = WindowProc;
  wc.hInstance     = GetModuleHandle(nullptr);
  wc.lpszClassName = s_windowClassName;

  // TODO: This should be adjustable.
  wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);

  if (RegisterClass(&wc) == 0) {
    winapiDie(L"RegisterClass");
  }

  s_windowClassRegistered = true;
}


void BaseWindow::createWindow(CreateWindowExWArgs const &origCW)
{
  assert(!m_hwnd);

  registerWindowClassIfNecessary();

  CreateWindowExWArgs cw(origCW);
  cw.m_lpClassName = s_windowClassName;
  cw.m_lpParam = this;

  HWND hwnd = cw.createWindow();
  if (!hwnd) {
    winapiDie(L"CreateWindowExW");
  }

  // `m_hwnd` should be set in `WindowProc`.
  assert(hwnd == m_hwnd);
}


/*static*/ LRESULT CALLBACK BaseWindow::WindowProc(
  HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  BaseWindow *pThis = nullptr;

  if (uMsg == WM_NCCREATE) {
    // Get the `BaseWindow` instance from `CREATESTRUCT` and save it in
    // `GWLP_USERDATA`.
    CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
    pThis = (BaseWindow*)(pCreate->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

    // The `WM_NCCREATE` message is delivered synchronously before
    // `CreateWindowExW` returns, so we need to set the handle here.
    pThis->m_hwnd = hwnd;
  }
  else {
    // Get the `BaseWindow` instance from `GWLP_USERDATA`.
    pThis = (BaseWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  }

  if (pThis) {
    return pThis->handleMessage(uMsg, wParam, lParam);
  }
  else {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}


LRESULT BaseWindow::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}


// EOF
