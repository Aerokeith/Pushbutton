#include <Arduino.h>
#include <elapsedMillis.h>

  // Default delay values; can be changed with setDelays()
const uint16_t defDebouncePeriod = 80;   // default switch debounce period (ms)
const uint16_t defDoubleTapDelay = 300;   // default max delay between first and second press (ms)
const uint16_t defLongPressDur = 1000;    // default min duration of long press (ms)

  /* Pushbutton switch states:
      RDY: Waiting for new button press
      WAIT_LONG: Button pressed, waiting for long-press duration or for button to go inactive before possible 2nd tap
      WAIT_DOUBLE: Button released, waiting for possible 2nd tap 
      WAIT_INACTIVE: Waiting for button to be released before returning to RDY state
  */
enum stateEnum {RDY, WAIT_LONG, WAIT_DOUBLE, WAIT_INACTIVE};

  /* Pushbutton switch events:
      NO_PRESS: No event yet, or previous event was read/cleared
      SINGLE_TAP: Button was pressed once and released
      DOUBLE_TAP:  Button was pressed twice with required timing
      LONG_PRESS: Button was pressed once and held for required duration
  */
enum eventEnum {NO_PRESS = 0b000, SINGLE_TAP = 0b001, DOUBLE_TAP = 0b010, LONG_PRESS = 0b100};


class pushButtonClass {
  uint8_t activeLevel;  // logic level for button press (HIGH or LOW)
  stateEnum state;      // current state of the switch (see swStateEnum)
  eventEnum event;      // last switch event detected
  elapsedMillis delayTimer;   // timer used for double-tap and longpress delays
  elapsedMillis lockoutTimer; // timer used for pushbitton switch debouncing
  uint16_t debouncePeriod = defDebouncePeriod; // pushbutton switch debounce lockout period (ms)
  uint16_t doubleTapDelay = defDoubleTapDelay; // max delay between first and second press (ms)
  uint16_t longPressDuration = defLongPressDur; // min duration of long press (ms)
  bool buttonActive;  // current (debounced) level of the switch
  bool lockout; // true when switch is in debounce lockout period
  bool doubleTapEnabled;  // true if double-tap function has been enabled
  bool longPressEnabled;  // true when long-press function has been enabled
public:
  uint8_t pNum;       // pin number of pushbutton switch input
  void init(uint8_t ioPinNum, uint8_t actLevel, bool pullup, int eventSel);
  void setDelays(uint16_t dbPeriod, uint16_t doubleDly, uint16_t longDur);
  void update();
  bool singleTap();
  bool doubleTap();
  bool longPress();
  bool eventDetected();
  eventEnum getEvent();
};
