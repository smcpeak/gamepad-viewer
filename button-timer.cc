// button-timer.cc
// Code for `button-timer.h`.

// See license.txt for copyright and terms of use.

#include "button-timer.h"              // this module


ButtonTimer::ButtonTimer()
  : m_running(false),
    m_startMS(0)
{}


void ButtonTimer::startTimer(ClockValue currentMS)
{
  m_running = true;
  m_startMS = currentMS;
}


void ButtonTimer::possiblyExpire(ClockValue currentMS,
                                 ClockValue maxDurationMS)
{
  if (m_running) {
    if (elapsedMS(currentMS) > maxDurationMS) {
      m_running = false;
    }
  }
}


auto ButtonTimer::elapsedMS(ClockValue currentMS) const -> ClockValue
{
  if (m_running) {
    // This uses wraparound arithmetic, which should be fine.
    return currentMS - m_startMS;
  }
  else {
    return 0;
  }
}


// EOF
