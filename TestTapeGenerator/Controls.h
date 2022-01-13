#ifndef CONTROLS_H
#define CONTROLS_H

#include <RTC.h>
#define RTC_H //Bug in RTC.h
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
  caret = '\xFF';
  //    1234567890123456789012345678901234567890
  //   "   -2   .   1   .   0   .   1   .   2+  "

  index = 8 * value + 20;

}

void Beep(long ms = 30)
{
  digitalWrite(11, HIGH); // set buzzer on
  delay(ms);
  digitalWrite(11, LOW); // set buzzer OFF
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

template <class T>
class Timer
{
    long ms;
    T *t;
    bool enabled;
  public:
    long period;
    void enable(void) {
      enabled = true;
      ms = millis();
    }
    void disable(void) {
      enabled = false;
      ms = 0;
    }
    Timer(T *t_):  t(t_), enabled(false), ms(0), period(0)  {
    }

    typedef void (T::*TimerTypedef)(Timer*);
    TimerTypedef OnTimer;

    void process() {
      if (millis() - ms > period) {
        ms = millis();
        if (OnTimer) {
          (t->*OnTimer)(this);
        }
      }
    }
};


template <class T>
class ButtonPanel
{
  protected:
    enum Buttons {BN_UP, BN_DOWN, BN_RIGHT, BN_LEFT, BN_PAGEUP, BN_PAGEDOWN, BN_ESCAPE, BN_OK};
    T *t;
  public:
    bool Execute()
    {
      auto pinl = PINL;
      int keycount = 0;

      do {
        if (OnUpdate) {
          (t->*OnUpdate)(this);
        }
        do {
          if (PINL) {
            if (pinl == PINL) {
              keycount++;
            } else {
              keycount = 1;
            }
            pinl = PINL;
          } else {
            keycount = 0;
            pinl = 0;
            if (OnLoop) {
              (t->*OnLoop)(this);
            }
          }
          delay(50);
        } while (keycount == 0 || ((keycount > 1) && (keycount < 4)));
        std::bitset<8> b{pinl};
        if (b.test(BN_UP)) {
          if (OnButtonUp) {
            (t->*OnButtonUp)(this);
          }
          Beep(25);
        } else if (b.test(BN_DOWN)) {
          if (OnButtonDown) {
            (t->*OnButtonDown)(this);
          }
          Beep(25);
        } else if (b.test(BN_RIGHT)) {
          if (OnButtonRight) {
            (t->*OnButtonRight)(this);
          }
          Beep(25);
        } else if (b.test(BN_LEFT)) {
          if (OnButtonLeft) {
            (t->*OnButtonLeft)(this);
          }
          Beep(25);
        } else if (b.test(BN_PAGEUP)) {
          if (OnButtonPageUp) {
            (t->*OnButtonPageUp)(this);
          }
          Beep(25);
        } else if (b.test(BN_PAGEDOWN)) {
          if (OnButtonPageDown) {
            (t->*OnButtonPageDown)(this);
          }
          Beep(25);
        } else if (b.test(BN_ESCAPE)) {
          if (OnButtonEscape) {
            (t->*OnButtonEscape)(this);
          }
          Beep(25);
          return false;
        } else if (b.test(BN_OK)) {
          if (OnButtonOk) {
            (t->*OnButtonOk)(this);
          }
          Beep(25);
          return true;
        }
      } while (true);
    }

    ButtonPanel(T *t_): t(t_),
      OnButtonUp(0),
      OnButtonDown(0),
      OnButtonRight(0),
      OnButtonLeft(0),
      OnButtonPageUp(0),
      OnButtonPageDown(0),
      OnButtonEscape(0),
      OnButtonOk(0),
      OnUpdate(0),
      OnLoop(0)
    {
    }
    typedef void (T::*MyTypedef)(ButtonPanel*);
    MyTypedef OnButtonUp;
    MyTypedef OnButtonDown;
    MyTypedef OnButtonRight;
    MyTypedef OnButtonLeft;
    MyTypedef OnButtonPageUp;
    MyTypedef OnButtonPageDown;
    MyTypedef OnButtonEscape;
    MyTypedef OnButtonOk;
    MyTypedef OnUpdate;
    MyTypedef OnLoop;
};


class RTC_Helper
{
  public:
    DS3231 RTC;
    RTC_Helper() {
      RTC.begin();             // start RTC
      RTC.setHourMode(CLOCK_H24); // set til 24 timer
    }
    std::string ToString() {
      char stringbuffer[255];
      sprintf(stringbuffer, "%02i:%02i:%02i", (int)RTC.getHours(), (int)RTC.getMinutes(), (int)RTC.getSeconds());
      return stringbuffer;
    }
    std::string ToStringExt() {
      char stringbuffer[255];
      char degree = '\xDF';
      sprintf(stringbuffer, "%02i:%02i:%02i %02i C", (int)RTC.getHours(), (int)RTC.getMinutes(), (int)RTC.getSeconds(), (int)RTC.getTemp());
      stringbuffer[11] = degree;
      return stringbuffer;
    }
};

class Menu
{
  protected:
    Timer<Menu> TimerClock;
    Timer<Menu> TimerLCD;
    uint16_t Inc()
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

  public:
    ButtonPanel<Menu> buttonPanel;
    void OnTimerClock (Timer<Menu> *timer)
    {
      digitalWrite(8, HIGH);
      lcdhelper.lcd.setCursor(32, 0);
      lcdhelper.lcd.print(rtchelper.ToString().c_str());
    }
    void OnTimerLCD (Timer<Menu> *timer)
    {
      lcdhelper.lcd.setBacklight(LOW);  // SET LCD LYS ON / OFF
    }
    void OnButtonUp (ButtonPanel<Menu> *buttonPanel)
    {
      TimerLCD.enable();
      Inc();
    }
    void OnButtonDown (ButtonPanel<Menu> *buttonPanel)
    {
      TimerLCD.enable();
      Dec();
    }
    void OnButtonPageUp (ButtonPanel<Menu> *buttonPanel)
    {
      TimerLCD.enable();
      for (int i = 0; i != 10; ++i) {
        Inc();
      }
    }
    void OnButtonPageDown (ButtonPanel<Menu> *buttonPanel)
    {
      TimerLCD.enable();
      for (int i = 0; i != 10; ++i) {
        Dec();
      }
    }
    void OnUpdate (ButtonPanel<Menu> *buttonPanel)
    {
      if (Current != Display) {
        UpdateLCD();
        lcdhelper.line[0].resize(32, ' ');
        lcdhelper.line[0] += rtchelper.ToString();
        lcdhelper.Show();
        Display = Current;
      }
    }
    void OnLoop (ButtonPanel<Menu> *buttonPanel)
    {
      TimerClock.process();
      TimerLCD.process();
    }
    RTC_Helper rtchelper;
    uint16_t End;
    uint16_t Current;
    uint16_t Display;
    LCD_Helper lcdhelper;
    virtual void UpdateLCD() = 0;
    virtual Menu (uint16_t End_): End(End_), Current(0), Display(End_), lcdhelper(false), buttonPanel(this), TimerClock(this), TimerLCD(this)
    {
      buttonPanel.OnButtonUp = & Menu::OnButtonUp;
      buttonPanel.OnButtonDown = & Menu::OnButtonDown;
      buttonPanel.OnButtonPageUp = & Menu::OnButtonPageUp;
      buttonPanel.OnButtonPageDown = & Menu::OnButtonPageDown;
      buttonPanel.OnUpdate = & Menu::OnUpdate;
      buttonPanel.OnLoop = & Menu::OnLoop;

      TimerClock.period = 1000;
      TimerClock.OnTimer = & Menu::OnTimerClock;
      TimerClock.enable();

      TimerLCD.period = 120000;
      TimerLCD.OnTimer = & Menu::OnTimerLCD;
      TimerLCD.enable();

      DDRL = B00000000; // all inputs PORT-L D42 til D49
    }

    bool Execute()
    {
      return buttonPanel.Execute();
    }
};

class Dialog
{
  public:
    RTC_Helper rtchelper;
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
