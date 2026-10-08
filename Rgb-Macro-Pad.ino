/*
  Rgb-Macro-Pad firmware
  =======================
  17-key USB macro/numpad firmware for an Arduino-compatible USB HID board.

  Hardware represented by Macropad.sch:
    - 4 matrix rows
    - 5 matrix columns
    - 17 switches
    - one steering diode per switch
    - USB HID keyboard

  IMPORTANT:
  The repository schematic currently labels the MCU as
  "USB Keyboard MCU (VERIFY PART)" and does not specify MCU GPIO numbers.
  Therefore the GPIO definitions below are intentionally easy to change.

  Default pinout below targets an Arduino Pro Micro / ATmega32U4.
  Verify your actual MCU and PCB traces before flashing.

  Key layout:
       NumLock   /    *    -
       7         8    9    +
       4         5    6
       1         2    3    0    .    Enter

  The matrix itself is 4x5; Enter is shown as a fifth-column key in the
  schematic. If your physical PCB uses a different row/column assignment,
  change KEYMAP[] and/or the row/column pins below.

  No external library is required beyond Arduino's built-in Keyboard library.
*/

#include <Arduino.h>
#include <Keyboard.h>

// ---------------------------------------------------------------------------
// MATRIX GPIO — CHANGE THESE TO MATCH YOUR PCB
// ---------------------------------------------------------------------------

// Arduino Pro Micro example pins.
// Rows are driven one at a time; columns are read with internal pull-ups.
const uint8_t ROW_PINS[4] = {4, 5, 6, 7};
const uint8_t COL_PINS[5] = {8, 9, 10, 14, 15};

// ---------------------------------------------------------------------------
// KEYMAP
// ---------------------------------------------------------------------------
//
// Matrix position -> HID key.
// Use KEY_NONE for unused matrix positions.
//
// Row 0: NumLock, /, *, -, unused
// Row 1: 7,       8, 9, +, unused
// Row 2: 4,       5, 6, unused, unused
// Row 3: 1,       2, 3, 0, ., Enter
//
// Because the physical design has 17 keys but only 20 matrix positions,
// the final three positions are unused.
//
// If your board's Enter key is wired differently, move KEY_ENTER to the
// correct row/column here.

const uint8_t KEY_NONE = 0x00;

const uint8_t KEYMAP[4][5] = {
  { KEY_NUM_LOCK, '/', '*', '-', KEY_NONE },
  { '7',          '8', '9', '+', KEY_NONE },
  { '4',          '5', '6', KEY_NONE, KEY_NONE },
  { '1',          '2', '3', '0', '.' }
};

// ---------------------------------------------------------------------------
// DEBOUNCE
// ---------------------------------------------------------------------------

const uint16_t DEBOUNCE_MS = 8;

bool stableState[4][5];
bool lastReading[4][5];
uint32_t lastChange[4][5];

// ---------------------------------------------------------------------------
// INITIALIZATION
// ---------------------------------------------------------------------------

void setup() {
  // Rows idle HIGH.
  for (uint8_t r = 0; r < 4; r++) {
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], HIGH);
  }

  // Columns use pull-ups. A pressed key pulls its column LOW when
  // its row is driven LOW.
  for (uint8_t c = 0; c < 5; c++) {
    pinMode(COL_PINS[c], INPUT_PULLUP);
  }

  for (uint8_t r = 0; r < 4; r++) {
    for (uint8_t c = 0; c < 5; c++) {
      stableState[r][c] = false;
      lastReading[r][c] = false;
      lastChange[r][c] = 0;
    }
  }

  Keyboard.begin();
  delay(200);
}

// ---------------------------------------------------------------------------
// MATRIX SCAN
// ---------------------------------------------------------------------------

void scanMatrix() {
  for (uint8_t r = 0; r < 4; r++) {

    // Drive only the active row LOW.
    digitalWrite(ROW_PINS[r], LOW);
    delayMicroseconds(30);

    for (uint8_t c = 0; c < 5; c++) {

      // LOW means switch is closed.
      bool pressed = (digitalRead(COL_PINS[c]) == LOW);

      if (pressed != lastReading[r][c]) {
        lastReading[r][c] = pressed;
        lastChange[r][c] = millis();
      }

      if ((millis() - lastChange[r][c]) >= DEBOUNCE_MS &&
          stableState[r][c] != pressed) {

        stableState[r][c] = pressed;
        handleKey(r, c, pressed);
      }
    }

    digitalWrite(ROW_PINS[r], HIGH);
  }
}

// ---------------------------------------------------------------------------
// KEY HANDLING
// ---------------------------------------------------------------------------

void handleKey(uint8_t row, uint8_t col, bool pressed) {
  uint8_t key = KEYMAP[row][col];

  if (key == KEY_NONE) {
    return;
  }

  if (pressed) {
    Keyboard.press(key);
  } else {
    Keyboard.release(key);
  }
}

// ---------------------------------------------------------------------------
// MAIN LOOP
// ---------------------------------------------------------------------------

void loop() {
  scanMatrix();
  delayMicroseconds(500);
}
