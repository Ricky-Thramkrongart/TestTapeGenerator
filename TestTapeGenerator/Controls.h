#pragma once

#include "LCDHelper.h"
#include "RTCHelper.h"
#include "Timer.h"
#include "Button.h"

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

std::vector<std::string> GetVUMeterStrings(double left, double right)
{
    std::vector<std::string> strs(3);
    double min_val = -2.0;
    double max_val = 2.0;
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

    virtual void OnTimerClock(Timer<BasePanel>* timer)
    {
        digitalWrite(8, HIGH);
        lcdhelper.lcd.setCursor(32, 0);
        lcdhelper.lcd.print(rtchelper.ToString());
    }
    virtual void OnTimerLCD(Timer<BasePanel>* timer)
    {
        lcdhelper.lcd.setBacklight(LOW);  // SET LCD LYS ON / OFF
    }
    virtual void OnButtonPressed(ButtonPanel<BasePanel>* buttonPanel)
    {
        lcdhelper.lcd.setBacklight(HIGH);  // SET LCD LYS ON / OFF
        TimerLCD.enable();
    }
    virtual void OnButtonUp(ButtonPanel<BasePanel>* buttonPanel)
    {
    }
    virtual void OnButtonDown(ButtonPanel<BasePanel>* buttonPanel)
    {
    }
    virtual void OnButtonRight(ButtonPanel<BasePanel>* buttonPanel)
    {
    }
    virtual void OnButtonLeft(ButtonPanel<BasePanel>* buttonPanel)
    {
    }
    virtual void OnButtonPageUp(ButtonPanel<BasePanel>* buttonPanel)
    {
    }
    virtual void OnButtonPageDown(ButtonPanel<BasePanel>* buttonPanel)
    {
    }
    virtual void OnButtonEscape(ButtonPanel<BasePanel>* buttonPanel)
    {
    }
    virtual void OnButtonOk(ButtonPanel<BasePanel>* buttonPanel)
    {
    }
    virtual void OnUpdate(ButtonPanel<BasePanel>* buttonPanel) {
        FullUpdate();
        lcdhelper.Line(0, 32, rtchelper.ToString());
        lcdhelper.Show();
    }
    virtual void OnLoop(ButtonPanel<BasePanel>* buttonPanel)
    {
        TimerClock.process();
        TimerLCD.process();
    }
    virtual void FullUpdate() = 0;
    BasePanel() : lcdhelper(false), buttonPanel(this), TimerClock(this), TimerLCD(this)
    {
        TimerClock.period = 1000;
        TimerClock.OnTimer = &BasePanel::OnTimerClock;
        TimerClock.enable();

        TimerLCD.period = 240000;
        TimerLCD.OnTimer = &BasePanel::OnTimerLCD;
        TimerLCD.enable();
    }

};