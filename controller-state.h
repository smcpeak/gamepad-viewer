// controller-state.h
// Defines `ControllerState`, encapsulating controller input data.

#ifndef CONTROLLER_STATE_H
#define CONTROLLER_STATE_H

#include "gpv-config.h"                // AnalogThresholdConfig

#include <xinput.h>                    // XINPUT_STATE


// Encapsulate the state of the controller and a few related variables.
class ControllerState {
public:      // data
  // Controller input state.
  XINPUT_STATE m_inputState;

  // True if 'm_controllerState' holds valid values.
  bool m_hasInputState;

  // Value of `GetTickCount()` when the input was read.
  DWORD m_pollTimeMS;

public:      // funcs
  ControllerState();

  ControllerState &operator=(ControllerState const &obj);

  // Read the controller state.
  void poll(int controllerID);

  // Return true if a trigger (which one depends on `leftSide`) should
  // be regarded as in a "depressed" state based on `atConfig`.
  bool isTriggerPressed(AnalogThresholdConfig const &atConfig,
                        bool leftSide) const;
};


#endif // CONTROLLER_STATE_H
