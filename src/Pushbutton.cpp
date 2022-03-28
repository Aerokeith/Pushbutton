/* PUSHBUTTON - THis library contains functions to detect "events" associated with an SPST-NO momentary-action pushbutton switch.

    pushbutton pb(pinNum, actLevel, pullup);
      Create a pushbutton object for a single switch, with the following parameters:
        pinNum: Arduino I/O pin number to which the pushbutton is connected
        actLevel: logic level for button press (LOW or HIGH)
        pullup: when true, enables the internal pullup resistor

    pb.enableEvents(eventSel);
      Used to enable the optional "double-tap" and/or "long-press" switch events. The "single-tap" event is always enabled.
      The optional events are specified using the following bit field parameter:
        eventSel: (DOUBLE_TAP) or (LONG_PRESS) or (DOUBLE_TAP | LONG_PRESS)

    pb.setDelays(dbPeriod, doubleDly, longDur);
      Used to override the default timing values used for swtch debouncing and event detection. 0 values are ignored and the 
      corresponding default is not changed. 
        dbPeriod: Pushbutton switch debounce lockout period (ms)
        doubleDly: Max delay between first and second press (ms)
        longDur: Min duration of long press (ms)

    pb.update();
      This function should must called periodically to monitor the pushbutton switch and detect one of the possible events
      defined by eventEnum (in Pushbutton.h). The interval between calls should be less than the debounce period (100ms nominal).

    pb.singleTap();
      Returns true if the periodically-called pb.update() function has detected a single-tap event. A true value is returned only 
      once for each event.

    pb.doubleTap();
      Returns true if the periodically-called pb.update() function has detected a double-tap event. A true value is returned only 
      once for each event. By default, the maximum interval between successive button presses (taps) is 300ms.

    pb.longPress();
      Returns true if the periodically-called pb.update() function has detected a long-press event. A true value is returned only 
      once for each event. By default, a long-press event is detected when the putton is depressed for at least 1000ms. 

    pb.eventDetected();
      Returns true if the periodically-called pb.update() function has detected any type of putton-press event, and the event has not 
      been cleared by a call to singleTap(), doubleTap(), longPress(), or getEvent(). This call does not clear the event.

    pb.getEvent();
      Returns one of the following state values: NO_PRESS, SINGLE_TAP, DOUBLE_TAP, or LONG_PRESS. The event is cleared and subsequent 
      calls will return NO_PRESS until a new event is detected.
*/

#include <Arduino.h>
#include "Pushbutton.h"


/* Pushbutton() is the object constructor. It intializes the pushbutton switch inputs and associated state variables. It accepts
    three parameters:
      pinNum: Arduino I/O pin number to which the pushbutton is connected
      actLevel: logic level for putton press (LOW or HIGH)
      pullup: when true, enables the internal pullup resistor
*/
pushbutton::pushbutton(uint8_t pinNum, uint8_t actLevel, bool pullup) {
  pb.enablePullup = pullup;
  pb.pinNum = pinNum;
  pb.activeLevel = actLevel;
  pb.state = UNINIT;    // switch remains uninitialized until first call to update()
  pb.event = NO_PRESS;
  pb.lockout = false;
  pb.doubleTapEnabled = false;
  pb.longPressEnabled = false;
}


/* enableEvents() is called to enable the double-tap and/or long-press functionality of the pushbutton switch
*/
void pushbutton::enableEvents(int eventSel) {
  pb.doubleTapEnabled = (eventSel & DOUBLE_TAP);
  pb.longPressEnabled = (eventSel & LONG_PRESS);
}


/* setDelays() can be called to override the default timing values. 0 values are ignored and the corresponding default is not changed.
*/
void pushbutton::setDelays(uint16_t dbPeriod, uint16_t doubleDly, uint16_t longDur) {
  if (dbPeriod > 0)
    pb.debouncePeriod = dbPeriod;
  if (doubleDly > 0)
    pb.doubleTapDelay = doubleDly;
  if (longDur > 0)
    pb.longPressDuration = longDur;
}


/* update() is called periodically to monitor a pushbutton switch and detect one of the possible events
    defined by eventEnum (in Pushbutton.h). The interval between calls should be less than the debounce period (100ms nominal)
*/
void pushbutton::update() {
  if (pb.state == UNINIT) {   // if this is the first call to update()
    pinMode(pb.pinNum, (pb.enablePullup? INPUT_PULLUP: INPUT)); // configure the input pin
    pb.state = RDY; // now ready to read pin
  }
  if (pb.lockout) {   // if pushbutton is currently in debounce lockout period
    if (pb.lockoutTimer > pb.debouncePeriod)  // if debounce period expired
      pb.lockout = false;   // end lockout, handle other actions in next call to update()
  }
  else {  // not in debounce lockout period
    pb.active = (digitalReadFast(pb.pinNum) == pb.activeLevel);  // get current pushbutton state (active or not)
    switch (pb.state) {   // actions depend on current state
      case RDY:   // waiting for switch press
        if (pb.active) {  // button was pressed
          pb.lockout = true;  // start lockout period
          pb.lockoutTimer = 0;  // start lockout period
          pb.delayTimer = 0;  // start delay timer for other possible actions
          if (pb.doubleTapEnabled || pb.longPressEnabled)   // if either of these functions are enabled
            pb.state = WAIT_LONG;   // transition to the next state, used by both functions
          else {  // neither function is enabled
            pb.event = SINGLE_TAP;  // record the press event immediately (no delays to wait for possible long- or double-)
            pb.state = WAIT_INACTIVE;   // go to this state to wait for switch release
          }
        } 
      break;
      case WAIT_LONG:   // button was pressed and either double-tap or long-press functions are enabled
        if (pb.active) {  // if switch is still active (not yet released)
          if (pb.longPressEnabled) {
            if (pb.delayTimer > pb.longPressDuration) {   // if long-press delay has expired
              pb.event = LONG_PRESS;  // record the event
              pb.state = WAIT_INACTIVE;   // go to this state to wait for button release
            }
          }
        }
        else {  // switch was just released
          pb.lockout = true;  // start debounce lockout period
          pb.lockoutTimer = 0;  
          if (pb.doubleTapEnabled)  // if this function is enabled
            pb.state = WAIT_DOUBLE; // transition to this state to wait for possible second press
          else {  // double-tap not enabled
            pb.event = SINGLE_TAP;  // it was just a single-tap; report immediately without waiting for end of release debounce
            pb.state = RDY;   // go to RDY state and wait for end of (release) debounce period
          }
        }
      break;
      case WAIT_DOUBLE: // button was pressed and released, now waiting for possible second press (after debounce)
        if (pb.delayTimer > pb.doubleTapDelay) {  // end of waiting period for double-tap
          pb.event = SINGLE_TAP;  // it was just a single-tap
          pb.state = RDY;   // // go to ready state (but note that release debounce lockout was previously started)
        }
        else {  // double-tap delay hasn't ended
          if (pb.active) {  // button pressed again within double-tap period
            pb.lockout = true;    // start debounce lockout
            pb.lockoutTimer = 0;
            pb.event = DOUBLE_TAP;    // record double-tap event
            pb.state = WAIT_INACTIVE; // go to this state to wait for button release
          }
        }
      break;
      case WAIT_INACTIVE: // waiting for button to be released before returning to RDY state
        if (!pb.active) {   // switch was released
          pb.lockout = true;    // start debounce lockout
          pb.lockoutTimer = 0;
          pb.state = RDY;   // return to ready state
        }
      break;
      default:
      break;
    }
  }
}


/* singleTap() returns true if the periodically-called update() function has detected a single-tap event. The state
    variable pb.event is cleared, so singleTap() will return true only once for each event.
*/
bool pushbutton::singleTap() {
  if (pb.event == SINGLE_TAP) {
    pb.event = NO_PRESS;
    return (true);
  }
  else 
    return (false);
}


/* doubleTap() returns true if the periodically-called update() function has detected a double-tap event. The state
    variable pb.event is cleared, so doubleTap() will return true only once for each event.
*/
bool pushbutton::doubleTap() {
  if (pb.event == DOUBLE_TAP) {
    pb.event = NO_PRESS;
    return (true);
  }
  else 
    return (false);
}


/* longPress() returns true if the periodically-called update() function has detected a long-press event. The state
    variable pb.event is cleared, so longPress() will return true only once for each event.
*/
bool pushbutton::longPress() {
  if (pb.event == LONG_PRESS) {
    pb.event = NO_PRESS;
    return (true);
  }
  else 
    return (false);
}


/* eventDetected() returns true if the periodically-called update() function has detected any type of putton-press event, and the
    event has not been cleared by a call to singleTap(), doubleTap(), longPress(), or getEvent(). This call does not clear the event.
*/
bool pushbutton::eventDetected() {
  return (pb.event != NO_PRESS);
}


/* getEvent() returns the current value of the pb.event state property and clears it. 
*/
eventEnum pushbutton::getEvent() {
  eventEnum event;

  event = pb.event;
  pb.event = NO_PRESS;
  return (event);
}