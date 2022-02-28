#pragma once

#include "LCDHelper.h"
#include "RTCHelper.h"
#include "Timer.h"
#include "Button.h"

String StatusControl(double treshhold, double LeftLevelDiff, double RightLevelDiff)
{
    char stringbuffer[] = "=:=";
    if (LeftLevelDiff < -treshhold)
    {
        stringbuffer[0] = '-';
    }
    if (LeftLevelDiff > treshhold)
    {
        stringbuffer[0] = '+';
    }

    if (RightLevelDiff < -treshhold)
    {
        stringbuffer[2] = '-';
    }
    if (RightLevelDiff > treshhold)
    {
        stringbuffer[2] = '+';
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

void GetVUMeterStrings(double left, double right, LCD_Helper & lcdhelper)
{
    char strs[3][41] = {
        {"   -2   .   1   .   0   .   1   .   2+  "},
        {"L                                       "},
        {"R                                       "} };
    
    double min_val = -2.0;
    double max_val = 2.0;
    //         1234567890123456789012345678901234567890

    char caret;
    int index;
    GetCaret(min_val, max_val, left, caret, index);
    strs[1][index] = caret;
    GetCaret(min_val, max_val, right, caret, index);
    strs[2][index] = caret;

    lcdhelper.Line(1, strs[0]);
    lcdhelper.Line(2, strs[1]);
    lcdhelper.Line(3, strs[2]);
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
        lcdhelper.lcd.setCursor(35, 0);
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
        lcdhelper.Line(0, 35, rtchelper.ToString());
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