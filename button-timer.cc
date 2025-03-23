// button-timer.cc
// Code for `button-timer.h`.

// See license.txt for copyright and terms of use.

#include "button-timer.h"              // this module


ButtonTimer::ButtonTimer()
  : m_running(false),
    m_startMS(0),
    m_queued(false)
{}


void ButtonTimer::startTimer(ClockValue currentMS)
{
  m_running = true;
  m_startMS = currentMS;
}


void ButtonTimer::startOrEnqueueTimer(ClockValue currentMS)
{
  if (!m_running) {
    startTimer(currentMS);
  }
  else {
    // It's fine if an input is already queued; only one can be queued
    // at a time.
    m_queued = true;
  }
}


void ButtonTimer::possiblyExpire(ClockValue currentMS,
                                 ClockValue maxDurationMS,
                                 ClockValue queuedStartMS)
{
  if (m_running) {
    if (elapsedMS(currentMS) > maxDurationMS) {
      if (!m_queued) {
        m_running = false;
      }
      else {
        // Consume the queued input.
        m_queued = false;

        // Set the elapsed time to `queuedStartMS`.
        m_startMS = currentMS - queuedStartMS;
      }
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
