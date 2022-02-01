#pragma once

#include "Controls.h"

class Menu : public BasePanel
{
protected:
    uint16_t Inc()
    {
        if (Current + 1 < End) {
            return Current = Current + 1;
        }
        return Current;
    }
    uint16_t Dec()
    {
        if (Current > 0) {
            return Current = Current - 1;
        }
        return Current;
    }

public:
    virtual void OnButtonUp(ButtonPanel<BasePanel>* buttonPanel)
    {
        BasePanel::OnButtonUp(buttonPanel);
        Inc();
    }
    virtual void OnButtonDown(ButtonPanel<BasePanel>* buttonPanel)
    {
        BasePanel::OnButtonDown(buttonPanel);
        Dec();
    }
    virtual void OnButtonPageUp(ButtonPanel<BasePanel>* buttonPanel)
    {
        BasePanel::OnButtonPageUp(buttonPanel);
        for (int i = 0; i != 10; ++i) {
            Inc();
        }
    }
    virtual void OnButtonPageDown(ButtonPanel<BasePanel>* buttonPanel)
    {
        BasePanel::OnButtonPageDown(buttonPanel);
        for (int i = 0; i != 10; ++i) {
            Dec();
        }
    }
    virtual void OnUpdate(ButtonPanel<BasePanel>* buttonPanel)
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
    Menu(uint16_t End_) : End(End_), Current(0), Display(End_)
    {
        buttonPanel.OnButtonPressed = &BasePanel::OnButtonPressed;
        buttonPanel.OnButtonUp = &BasePanel::OnButtonUp;
        buttonPanel.OnButtonDown = &BasePanel::OnButtonDown;
        buttonPanel.OnButtonPageUp = &BasePanel::OnButtonPageUp;
        buttonPanel.OnButtonPageDown = &BasePanel::OnButtonPageDown;
        buttonPanel.OnUpdate = &BasePanel::OnUpdate;
        buttonPanel.OnLoop = &BasePanel::OnLoop;
    }

    virtual bool Execute()
    {
        return buttonPanel.Execute();
    }
};