// controller-state.cc
// Code for controller-state.h.

#include "controller-state.h"          // this module

#include <windows.h>                   // GetTickCount
#include <xinput.h>                    // XInputGetState

#include <cstring>                     // std::{memset, memcpy}


ControllerState::ControllerState()
  : m_inputState(),
    m_hasInputState(false),
    m_pollTimeMS(0)
{
  // I'm not sure if the default ctor initializes this.
  std::memset(&m_inputState, 0, sizeof(m_inputState));
}


ControllerState &ControllerState::operator=(ControllerState const &obj)
{
  std::memcpy(&m_inputState,
              &obj.m_inputState,
              sizeof(m_inputState));

  m_hasInputState = obj.m_hasInputState;
  m_pollTimeMS = obj.m_pollTimeMS;

  return *this;
}


void ControllerState::poll(int controllerID)
{
  std::memset(&m_inputState, 0, sizeof(m_inputState));
  DWORD res = XInputGetState(controllerID, &m_inputState);
  m_hasInputState = (res == ERROR_SUCCESS);

  m_pollTimeMS = GetTickCount();
}


bool ControllerState::isTriggerPressed(
  AnalogThresholdConfig const &atConfig,
  bool leftSide) const
{
  if (m_hasInputState) {
    BYTE trigger = (leftSide? m_inputState.Gamepad.bLeftTrigger :
                              m_inputState.Gamepad.bRightTrigger);

    return trigger > atConfig.m_triggerDeadZone;
  }

  else {
    return false;
  }
}


// EOF
