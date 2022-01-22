#ifndef CONTROLS_H
#define CONTROLS_H

#include "LCDHelper.h"
#include <DS3232RTC.h>

std::string StatusControl(double trashhold, uint16_t LeftLevel, uint16_t RightLevel)
{
    char stringbuffer[] = "=:=";
    if (LeftLevel < -trashhold)
    {
        stringbuffer[0] = '<';
    }
    if (LeftLevel > trashhold)
    {
        stringbuffer[0] = '>';
    }

    if (RightLevel < -trashhold)
    {
        stringbuffer[2] = '<';
    }
    if (RightLevel > trashhold)
    {
        stringbuffer[2] = '>';
    }

    return stringbuffer;
}

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
                        if (OnButtonPressed) {
                            (t->*OnButtonPressed)(this);
                        }
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
            OnButtonPressed(0),
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
            DDRL = B00000000; // all inputs PORT-L D42 til D49
        }
        typedef void (T::*MyTypedef)(ButtonPanel*);
        MyTypedef OnButtonPressed;
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
        RTC_Helper() {
            RTC.begin();
            setSyncProvider(RTC.get);   // the function to get the time from the RTC
        }
        std::string ToString() {
            char stringbuffer[255];
            sprintf(stringbuffer, "%02i:%02i:%02i", hour(), minute(), second());
            return stringbuffer;
        }
        std::string ToStringExt() {
            char stringbuffer[255];
            char degree = '\xDF';
            sprintf(stringbuffer, "%02i:%02i:%02i %02i C", hour(), minute(), second(), RTC.temperature());
            stringbuffer[11] = degree;
            return stringbuffer;
        }
};


class BasePanel
{
    protected:
        Timer<BasePanel> TimerClock;
        Timer<BasePanel> TimerLCD;
    public:
        RTC_Helper rtchelper;
        LCD_Helper lcdhelper;

    public:
        ButtonPanel<BasePanel> buttonPanel;

        virtual void OnTimerClock (Timer<BasePanel> *timer)
        {
            digitalWrite(8, HIGH);
            lcdhelper.lcd.setCursor(32, 0);
            lcdhelper.lcd.print(rtchelper.ToString().c_str());
        }
        virtual void OnTimerLCD (Timer<BasePanel> *timer)
        {
            lcdhelper.lcd.setBacklight(LOW);  // SET LCD LYS ON / OFF
        }
        virtual void OnButtonPressed (ButtonPanel<BasePanel> *buttonPanel)
        {
            lcdhelper.lcd.setBacklight(HIGH);  // SET LCD LYS ON / OFF
            TimerLCD.enable();
        }
        virtual void OnButtonUp (ButtonPanel<BasePanel> *buttonPanel)
        {
        }
        virtual void OnButtonDown (ButtonPanel<BasePanel> *buttonPanel)
        {
        }
        virtual void OnButtonRight (ButtonPanel<BasePanel> *buttonPanel)
        {
        }
        virtual void OnButtonLeft (ButtonPanel<BasePanel> *buttonPanel)
        {
        }
        virtual void OnButtonPageUp (ButtonPanel<BasePanel> *buttonPanel)
        {
        }
        virtual void OnButtonPageDown (ButtonPanel<BasePanel> *buttonPanel)
        {
        }
        virtual void OnButtonEscape (ButtonPanel<BasePanel> *buttonPanel)
        {
        }
        virtual void OnButtonOk (ButtonPanel<BasePanel> *buttonPanel)
        {
        }
        virtual void OnUpdate (ButtonPanel<BasePanel> *buttonPanel) {
            FullUpdate();
            lcdhelper.line[0].resize(32, ' ');
            lcdhelper.line[0] += rtchelper.ToString();
            lcdhelper.Show();
        }
        virtual void OnLoop (ButtonPanel<BasePanel> *buttonPanel)
        {
            TimerClock.process();
            TimerLCD.process();
        }
        virtual void FullUpdate() = 0;
        BasePanel() : lcdhelper(false), buttonPanel(this), TimerClock(this), TimerLCD(this)
        {
            TimerClock.period = 1000;
            TimerClock.OnTimer = & BasePanel::OnTimerClock;
            TimerClock.enable();

            TimerLCD.period = 240000;
            TimerLCD.OnTimer = & BasePanel::OnTimerLCD;
            TimerLCD.enable();
        }

};

class Menu : public BasePanel
{
    protected:
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
        virtual void OnButtonUp (ButtonPanel<BasePanel> *buttonPanel)
        {
            BasePanel::OnButtonUp(buttonPanel);
            Inc();
        }
        virtual void OnButtonDown (ButtonPanel<BasePanel> *buttonPanel)
        {
            BasePanel::OnButtonDown(buttonPanel);
            Dec();
        }
        virtual void OnButtonPageUp (ButtonPanel<BasePanel> *buttonPanel)
        {
            BasePanel::OnButtonPageUp(buttonPanel);
            for (int i = 0; i != 10; ++i) {
                Inc();
            }
        }
        virtual void OnButtonPageDown (ButtonPanel<BasePanel> *buttonPanel)
        {
            BasePanel::OnButtonPageDown(buttonPanel);
            for (int i = 0; i != 10; ++i) {
                Dec();
            }
        }
        virtual void OnUpdate (ButtonPanel<BasePanel> *buttonPanel)
        {
            if (Current != Display) {
                BasePanel::OnUpdate(buttonPanel);
                Display = Current;
                char stringbuffer[255];
                sprintf(stringbuffer, "(%i/%i)", Current + 1, End);
                digitalWrite(8, LOW);
                lcdhelper.lcd.setCursor(33, 1);
                lcdhelper.lcd.print(stringbuffer);
            }
        }

        uint16_t End;
        uint16_t Current;
        uint16_t Display;
        virtual Menu (uint16_t End_): End(End_), Current(0), Display(End_)
        {
            buttonPanel.OnButtonPressed = & BasePanel::OnButtonPressed;
            buttonPanel.OnButtonUp = & BasePanel::OnButtonUp;
            buttonPanel.OnButtonDown = & BasePanel::OnButtonDown;
            buttonPanel.OnButtonPageUp = & BasePanel::OnButtonPageUp;
            buttonPanel.OnButtonPageDown = & BasePanel::OnButtonPageDown;
            buttonPanel.OnUpdate = & BasePanel::OnUpdate;
            buttonPanel.OnLoop = & BasePanel::OnLoop;
        }

        virtual bool Execute()
        {
            return buttonPanel.Execute();
        }
};

class DialogOk
{
    protected:
        Timer<DialogOk> TimerClock;
        bool Updated;

    public:
        ButtonPanel<DialogOk> buttonPanel;
        void OnUpdate (ButtonPanel<DialogOk> *buttonPanel)
        {
            if (!Updated) {
                FullUpdate();
                lcdhelper.line[0].resize(32, ' ');
                lcdhelper.line[0] += rtchelper.ToString();
                lcdhelper.Show();
                Updated = true;
            }
        }
        void OnLoop (ButtonPanel<DialogOk> *buttonPanel)
        {
            TimerClock.process();
            Update();
        }
        void OnTimerClock (Timer<DialogOk> *timer)
        {
            digitalWrite(8, HIGH);
            lcdhelper.lcd.setCursor(32, 0);
            lcdhelper.lcd.print(rtchelper.ToString().c_str());
        }
        RTC_Helper rtchelper;
        LCD_Helper lcdhelper;
        virtual void FullUpdate() = 0;
        virtual void Update() = 0;
        virtual DialogOk (void): lcdhelper(false), buttonPanel(this), TimerClock(this), Updated(false)
        {
            buttonPanel.OnUpdate = & DialogOk::OnUpdate;
            buttonPanel.OnLoop = & DialogOk::OnLoop;

            TimerClock.period = 1000;
            TimerClock.OnTimer = & DialogOk::OnTimerClock;
            TimerClock.enable();
        }

        bool Execute()
        {
            return buttonPanel.Execute();
        }
};

class Dialog
{
    public:
        LCD_Helper lcdhelper;
        bool finished;
        enum Buttons {BN_UP, BN_DOWN, BN_RIGHT, BN_LEFT, BN_PAGEUP, BN_PAGEDOWN, BN_ESCAPE, BN_OK};
        virtual void FullUpdate() = 0;
        uint16_t _delay;
        virtual Dialog (uint16_t delay_): _delay(delay_), finished(false)
        {
        }

        bool Execute()
        {
            if (!_delay) {
                FullUpdate();
                lcdhelper.Show();
            }
            auto pinl = PINL;
            do {
                do {
                    if (_delay) {
                        FullUpdate();
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
