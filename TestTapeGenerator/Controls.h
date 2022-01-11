#ifndef CONTROLS_H
#define CONTROLS_H

#include "LCDHelper.h"

void GetCaret(double min_val, double max_val, double value, char& caret, int& index)
{
  if (value < min_val) {
    caret = '<';
    index = 1;
    return;
  }
  if (value > max_val) {
    caret = '>';
    index = 39;
    return;
  }
  caret = '\xDB';
  //    1234567890123456789012345678901234567890
  //   "   -2   .   1   .   0   .   1   .   2+  "

  index = 8 * value + 20;

}

std::vector<std::string> GetVUMeterStrings (double left, double right)
{
  std::vector<std::string> strs(3);
  double min_val = -2.0;
  double max_val =  2.0;
  //         1234567890123456789012345678901234567890
  strs[0] = "   -2   .   1   .   0   .   1   .   2+  ";
  strs[1] = "L                                       ";
  strs[2] = "R                                       ";

  char caret;
  int index;
  GetCaret(min_val, max_val, left, caret, index);
  strs[1][index] = caret;
  GetCaret(min_val, max_val, right, caret, index);
  strs[2][index] = caret;
  return strs;
}

class Menu
{
  public:
    uint16_t End;
    uint16_t Current;
    uint16_t Display;
    LCD_Helper lcdhelper;
    enum Buttons {BN_UP, BN_DOWN, BN_RIGHT, BN_LEFT, BN_PAGEUP, BN_PAGEDOWN, BN_ESCAPE, BN_OK};
    virtual void UpdateLCD() = 0;
    virtual Menu (uint16_t End_): End(End_), Current(0), Display(End_), lcdhelper(false)
    {
      DDRL = B00000000; // all inputs PORT-L D42 til D49
    }

    uint16_t Inc ()
    {
      if (Current + 1 <  End) {
        return Current = Current + 1;
      }
      return Current;
    }
    uint16_t Dec ()
    {
      if (Current  > 0) {
        return Current = Current - 1;
      }
      return Current;
    }

    bool Execute()
    {
      auto pinl = PINL;
      do {
        if (Current != Display) {
          UpdateLCD();
          lcdhelper.Show();
          Display = Current;
        }

        do {
        } while ((pinl == PINL) || (pinl = PINL) == 0);
        enum Buttons {BN_UP, BN_DOWN, BN_RIGHT, BN_LEFT, BN_PAGEUP, BN_PAGEDOWN, BN_ESCAPE, BN_OK};

        std::bitset<8> b{pinl};
        if (b.test(BN_UP) || b.test(BN_RIGHT)) {
          Inc();
          delay(150);
        } else if (b.test(BN_DOWN) || b.test(BN_LEFT)) {
          Dec();
          delay(150);
        } else if (b.test(BN_PAGEUP)) {
          for (int i = 0; i != 10; ++i) {
            Inc();
          }
          delay(150);
        } else if (b.test(BN_PAGEDOWN)) {
          for (int i = 0; i != 10; ++i) {
            Dec();
          }
          delay(150);
        } else if (b.test(BN_ESCAPE)) {
          delay(150);
          return false;
        } else if (b.test(BN_OK)) {
          delay(150);
          return true;
        }
      } while (true);
    }
};

class Dialog
{
  public:
    LCD_Helper lcdhelper;
    bool finished;
    enum Buttons {BN_UP, BN_DOWN, BN_RIGHT, BN_LEFT, BN_PAGEUP, BN_PAGEDOWN, BN_ESCAPE, BN_OK};
    virtual void UpdateLCD() = 0;
    uint16_t _delay;
    virtual Dialog (uint16_t delay_): _delay(delay_), finished(false)
    {
      DDRL = B00000000; // all inputs PORT-L D42 til D49
    }

    bool Execute()
    {
      if (!_delay) {
        UpdateLCD();
        lcdhelper.Show();
      }
      auto pinl = PINL;
      do {
        do {
          if (_delay) {
            UpdateLCD();
            lcdhelper.Show();
            delay(1000);
            if (finished) {
              return true;
            }
          }
        } while ((pinl == PINL) || (pinl = PINL) == 0);
        enum Buttons {BN_UP, BN_DOWN, BN_RIGHT, BN_LEFT, BN_PAGEUP, BN_PAGEDOWN, BN_ESCAPE, BN_OK};

        std::bitset<8> b{pinl};
        if (b.test(BN_ESCAPE)) {
          delay(150);
          return false;
        } else if (b.test(BN_OK)) {
          delay(150);
          return true;
        }
      } while (true);
    }
};


#endif // CONTROLS_H
