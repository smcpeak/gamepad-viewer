// button-timer.h
// `ButtonTimer` class, which maintains a timer for a recent button press.

// See license.txt for copyright and terms of use.

#ifndef BUTTON_TIMER_H
#define BUTTON_TIMER_H

#include <cstdint>                     // std::uint64_t

// Timing state for a recent button press.
class ButtonTimer {
public:      // types
  // Milliseconds since an arbitrary point in the past.  Tacitly, I
  // expect this to represent a return value from win64
  // `GetTickCount()`, which is a `DWORD`, but I'd like this module to
  // not explicitly depend on `windows.h`.
  typedef std::uint64_t ClockValue;

public:      // data
  // True if the timer is running.  If the button has not been pressed
  // recently, then the timer is not running.
  //
  // Note: I avoid the term "active" to describe when the timer is
  // running because that term is instead used to refer to the portion
  // of time during which an Elden Ring parry would be able to deflect
  // an incoming attack (by default, between 200ms and 400ms after L2 is
  // pressed).
  bool m_running;

  // Clock value when the button was pressed.  This is only meaningful
  // if `m_running` is true.
  ClockValue m_startMS;

  // If true, the timer is running, and the user has enqueued another
  // input that should restart the timer immediately.
  bool m_queued;

public:      // funcs
  ButtonTimer();

  bool isRunning() const { return m_running; }

  // Set the timer running.
  void startTimer(ClockValue currentMS);

  // Start the timer, unless it is already running, in which case
  // enqueue another run after the current one finishes.
  void startOrEnqueueTimer(ClockValue currentMS);

  // If the timer has been running for more than `maxDurationMS`, set it
  // to the not-running state.
  //
  // However, if `m_queued`, then instead start another run with the
  // elapsed time as `queuedStartMS`.  The reason for the "queued start"
  // is it allows the enqueued action to be as if it started a little
  // bit in the past, which I use to skip the portion of the timer that
  // would otherwise account for input lag in the game.
  //
  void possiblyExpire(ClockValue currentMS, ClockValue maxDurationMS,
                      ClockValue queuedStartMS = 0);

  // Number of milliseconds since the timer started, or 0 if it is
  // not running.
  ClockValue elapsedMS(ClockValue currentMS) const;
};

#endif // BUTTON_TIMER_H
