/* PUSHBUTTON.CPP
    Implements a pushButtonClass that provides methods to detect "events" associated with an SPST-NO momentary-action 
    pushbutton switch:
      SINGLE_TAP: Button is pressed briefly, one time
      DOUBLE_TAP: Button is pressed twice in quick succession
      LONG_PRESS: Button is pressed and held for a minimumn duration
*/

#include <Arduino.h>
#include "Pushbutton.h"


/* pushButtonClass::init()
    Intializes the pushbutton switch input and associated state variables. 
    Parameters:
      uint8_t pinNum: Arduino I/O pin number to which the pushbutton is connected
      uint8_t actLevel: logic level for putton press (LOW or HIGH)
      bool pullup: when true, enables the internal pullup resistor
      int eventSel: bit mask used to enable events in additon to SINGLE_TAP (see eventEnum in Pushbutton.h)
    Returns: None
*/
void pushButtonClass::init(uint8_t ioPinNum, uint8_t actLevel, bool pullup, int eventSel) {
  pNum = ioPinNum;
  activeLevel = actLevel;
  pinMode(pNum, (pullup? INPUT_PULLUP: INPUT)); // configure the input pin
  state = RDY; 
  event = NO_PRESS;
  lockout = false;
  doubleTapEnabled = (eventSel & DOUBLE_TAP);
  longPressEnabled = (eventSel & LONG_PRESS);
}


/* pushButtonClass::setDelays()
    Used to override the default timing values used for swtch debouncing and event detection. 0 values are ignored and the 
      corresponding default is not changed. 
    Parameters:
      uint16_t dbPeriod: Pushbutton switch debounce lockout period (ms)
      uint16_t doubleDly: Max delay between first and second press (ms)
      uint16_t longDur: Min duration of long press (ms)
*/
void pushButtonClass::setDelays(uint16_t dbPeriod, uint16_t doubleDly, uint16_t longDur) {
  if (dbPeriod > 0)
    debouncePeriod = dbPeriod;
  if (doubleDly > 0)
    doubleTapDelay = doubleDly;
  if (longDur > 0)
    longPressDuration = longDur;
}


/* pushButtonClass::update()
    Called periodically to monitor a pushbutton switch and detect one of the possible events defined by eventEnum (in Pushbutton.h). 
    The interval between calls should be less than the debounce period (80ms by default)
*/
void pushButtonClass::update() {
  if (lockout) {   // if pushbutton is currently in debounce lockout period
    if (lockoutTimer > debouncePeriod)  // if debounce period expired
      lockout = false;   // end lockout, handle other actions in next call to update()
  }
  else {  // not in debounce lockout period
    buttonActive = (digitalReadFast(pNum) == activeLevel);  // get current pushbutton state (active or not)
    switch (state) {   // actions depend on current state
      case RDY:   // waiting for switch press
        if (buttonActive) {  // button was pressed
          lockout = true;  // start lockout period
          lockoutTimer = 0;  // start lockout period
          delayTimer = 0;  // start delay timer for other possible actions
          if (doubleTapEnabled || longPressEnabled)   // if either of these functions are enabled
            state = WAIT_LONG;   // transition to the next state, used by both functions
          else {  // neither function is enabled
            event = SINGLE_TAP;  // record the press event immediately (no delays to wait for possible long- or double-)
            state = WAIT_INACTIVE;   // go to this state to wait for switch release
          }
        } 
      break;
      case WAIT_LONG:   // button was pressed and either double-tap or long-press functions are enabled
        if (buttonActive) {  // if switch is still active (not yet released)
          if (longPressEnabled) {
            if (delayTimer > longPressDuration) {   // if long-press delay has expired
              event = LONG_PRESS;  // record the event
              state = WAIT_INACTIVE;   // go to this state to wait for button release
            }
          }
        }
        else {  // switch was just released
          lockout = true;  // start debounce lockout period
          lockoutTimer = 0;  
          if (doubleTapEnabled)  // if this function is enabled
            state = WAIT_DOUBLE; // transition to this state to wait for possible second press
          else {  // double-tap not enabled
            event = SINGLE_TAP;  // it was just a single-tap; report immediately without waiting for end of release debounce
            state = RDY;   // go to RDY state and wait for end of (release) debounce period
          }
        }
      break;
      case WAIT_DOUBLE: // button was pressed and released, now waiting for possible second press (after debounce)
        if (delayTimer > doubleTapDelay) {  // end of waiting period for double-tap
          event = SINGLE_TAP;  // it was just a single-tap
          state = RDY;   // // go to ready state (but note that release debounce lockout was previously started)
        }
        else {  // double-tap delay hasn't ended
          if (buttonActive) {  // button pressed again within double-tap period
            lockout = true;    // start debounce lockout
            lockoutTimer = 0;
            event = DOUBLE_TAP;    // record double-tap event
            state = WAIT_INACTIVE; // go to this state to wait for button release
          }
        }
      break;
      case WAIT_INACTIVE: // waiting for button to be released before returning to RDY state
        if (!buttonActive) {   // switch was released
          lockout = true;    // start debounce lockout
          lockoutTimer = 0;
          state = RDY;   // return to ready state
        }
      break;
      default:
      break;
    }
  }
}


/* pushButtonClass::singleTap() 
    Returns true if the periodically-called update() function has detected a single-tap event. The state
      variable "event" is cleared, so singleTap() will return true only once for each event.
    Parameters: None
    Returns: 
      bool: true (one time) if SINGLE_TAP event has been detected 
*/
bool pushButtonClass::singleTap() {
  if (event == SINGLE_TAP) {
    event = NO_PRESS;
    return (true);
  }
  else 
    return (false);
}


/* pushButtonClass::doubleTap() 
    returns true if the periodically-called update() function has detected a double-tap event. The state
      variable pb.event is cleared, so doubleTap() will return true only once for each event.
    Parameters: None
    Returns:
      bool: true (one time) if DOUBLE_TAP event has been detected
*/
bool pushButtonClass::doubleTap() {
  if (event == DOUBLE_TAP) {
    event = NO_PRESS;
    return (true);
  }
  else 
    return (false);
}


/* pushButtonClass::longPress() 
    returns true if the periodically-called update() function has detected a long-press event. The state
      variable pb.event is cleared, so longPress() will return true only once for each event.
    Parameters: None
    Returns:
      bool: true (one time) if LONG_PRESS event has bee detected
*/
bool pushButtonClass::longPress() {
  if (event == LONG_PRESS) {
    event = NO_PRESS;
    return (true);
  }
  else 
    return (false);
}


/* pushButtonClass::eventDetected() 
    returns true if the periodically-called update() function has detected any type of putton-press event, and the event has not 
      been cleared by a call to singleTap(), doubleTap(), longPress(), or getEvent(). This call does not clear the event.
    Parameters: None
    Returns:
      bool: true if any event has been detected
*/
bool pushButtonClass::eventDetected() {
  return (event != NO_PRESS);
}


/* pushButtonClass::getEvent() 
    returns the current value of the pb.event state variable and clears it. 
    Parameters: None
    Returns:
      eventEnum: current value of the pb.event state variable
*/
eventEnum pushButtonClass::getEvent() {
  eventEnum v;
  v = event;
  event = NO_PRESS;
  return (v);
}