#include <Arduino.h>
#include <elapsedMillis.h>

using namespace std;

  // Default delay values; can be changed with setDelays()
#define DEBOUNCE_PERIOD 80        // pushbutton switch debounce lockout period (ms)
#define DOUBLETAP_DELAY 300       // max delay between first and second press (ms)
#define LONGPRESS_DURATION 1000      // min duration of long press (ms)

  /* Pushbutton switch states:
      RDY: Waiting for new button press
      WAIT_LONG: Button pressed, waiting for long-press duration or for button to go inactive before possible 2nd tap
      WAIT_DOUBLE: Button released, waiting for possible 2nd tap 
      WAIT_INACTIVE: Waiting for button to be released before returning to RDY state
  */
enum stateEnum {RDY, WAIT_LONG, WAIT_DOUBLE, WAIT_INACTIVE};

  /* Pushbutton switch events:
      NONE: No event yet, or previous event was read/cleared
      SINGLE_TAP: Button was pressed once and released
      DOUBLE_TAP:  Button was pressed twice with required timing
      LONG_PRESS: Button was pressed once and held for required duration
  */
enum eventEnum {NO_PRESS = 0b000, SINGLE_TAP = 0b001, DOUBLE_TAP = 0b010, LONG_PRESS = 0b100};

struct pushbuttonStruct {
  uint8_t pinNum;       // pin number of pushbutton switch input
  uint8_t activeLevel;  // logic level for button press (HIGH or LOW)
  stateEnum state;  // current state of the switch (see swStateEnum)
  eventEnum event;  // last switch event detected
  elapsedMillis delayTimer;   // timer used for double-tap and longpress delays
  elapsedMillis lockoutTimer; // timer used for pushbitton switch debouncing
  uint16_t debouncePeriod = DEBOUNCE_PERIOD; // pushbutton switch debounce lockout period (ms)
  uint16_t doubleTapDelay = DOUBLETAP_DELAY; // max delay between first and second press (ms)
  uint16_t longPressDuration = LONGPRESS_DURATION; // min duration of long press (ms)
  bool active;  // current (debounced) level of the switch
  bool lockout; // true when switch is in debounce lockout period
  bool doubleTapEnabled;  // true if double-tap function has been enabled
  bool longPressEnabled;  // true when long-press function has been enabled
};


class pushbutton {
  pushbuttonStruct pb;
public:
  pushbutton(uint8_t pinNum, uint8_t actLevel, bool pullup);
  void enableEvents(int eventSel);
  void setDelays(int dbPeriod, int doubleDly, int longDur);
  void update();
  bool singleTap();
  bool doubleTap();
  bool longPress();
  bool eventDetected();
  eventEnum getEvent();
};
