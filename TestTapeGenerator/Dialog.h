#pragma once

#include "LCDHelper.h"
#include "Button.h"

class DialogOk
{
protected:
    Timer<DialogOk> TimerClock;
    bool Updated;

public:
    ButtonPanel<DialogOk> buttonPanel;
    void OnUpdate(ButtonPanel<DialogOk>* buttonPanel)
    {
        if (!Updated) {
            FullUpdate();
            lcdhelper.line[0].resize(32, ' ');
            lcdhelper.line[0] += rtchelper.ToString();
            lcdhelper.Show();
            Updated = true;
        }
    }
    void OnLoop(ButtonPanel<DialogOk>* buttonPanel)
    {
        TimerClock.process();
        Update();
    }
    void OnTimerClock(Timer<DialogOk>* timer)
    {
        digitalWrite(8, HIGH);
        lcdhelper.lcd.setCursor(32, 0);
        lcdhelper.lcd.print(rtchelper.ToString().c_str());
    }
    RTC_Helper rtchelper;
    LCD_Helper lcdhelper;
    virtual void FullUpdate() = 0;
    virtual void Update() = 0;
    virtual DialogOk(void) : lcdhelper(false), buttonPanel(this), TimerClock(this), Updated(false)
    {
        buttonPanel.OnUpdate = &DialogOk::OnUpdate;
        buttonPanel.OnLoop = &DialogOk::OnLoop;

        TimerClock.period = 1000;
        TimerClock.OnTimer = &DialogOk::OnTimerClock;
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
    enum Buttons { BN_UP, BN_DOWN, BN_RIGHT, BN_LEFT, BN_PAGEUP, BN_PAGEDOWN, BN_ESCAPE, BN_OK };
    virtual void FullUpdate() = 0;
    uint16_t _delay;
    virtual Dialog(uint16_t delay_) : _delay(delay_), finished(false)
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

            std::bitset<8> b{ pinl };
            if (b.test(BN_ESCAPE)) {
                delay(150);
                return false;
            }
            else if (b.test(BN_OK)) {
                delay(150);
                return true;
            }
        } while (true);
    }
};