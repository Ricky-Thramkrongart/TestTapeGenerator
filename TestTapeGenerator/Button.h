#pragma once

#include <ArduinoSTL.h>
#include <bitset>
#include "Beep.h"

template <class T>
class ButtonPanel
{
protected:
    enum Buttons { BN_UP, BN_DOWN, BN_RIGHT, BN_LEFT, BN_PAGEUP, BN_PAGEDOWN, BN_ESCAPE, BN_OK };
    T* t;
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
                    }
                    else {
                        keycount = 1;
                    }
                    pinl = PINL;
                    if (OnButtonPressed) {
                        (t->*OnButtonPressed)(this);
                    }
                }
                else {
                    keycount = 0;
                    pinl = 0;
                    if (OnLoop) {
                        (t->*OnLoop)(this);
                    }
                }
                delay(50);
            } while (keycount == 0 || ((keycount > 1) && (keycount < 4)));
            std::bitset<8> b{ pinl };
            if (b.test(BN_UP)) {
                if (OnButtonUp) {
                    (t->*OnButtonUp)(this);
                }
                Beep(25);
            }
            else if (b.test(BN_DOWN)) {
                if (OnButtonDown) {
                    (t->*OnButtonDown)(this);
                }
                Beep(25);
            }
            else if (b.test(BN_RIGHT)) {
                if (OnButtonRight) {
                    (t->*OnButtonRight)(this);
                }
                Beep(25);
            }
            else if (b.test(BN_LEFT)) {
                if (OnButtonLeft) {
                    (t->*OnButtonLeft)(this);
                }
                Beep(25);
            }
            else if (b.test(BN_PAGEUP)) {
                if (OnButtonPageUp) {
                    (t->*OnButtonPageUp)(this);
                }
                Beep(25);
            }
            else if (b.test(BN_PAGEDOWN)) {
                if (OnButtonPageDown) {
                    (t->*OnButtonPageDown)(this);
                }
                Beep(25);
            }
            else if (b.test(BN_ESCAPE)) {
                if (OnButtonEscape) {
                    (t->*OnButtonEscape)(this);
                }
                Beep(25);
                return false;
            }
            else if (b.test(BN_OK)) {
                if (OnButtonOk) {
                    (t->*OnButtonOk)(this);
                }
                Beep(25);
                return true;
            }
        } while (true);
    }

    ButtonPanel(T* t_) : t(t_),
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
    typedef void (T::* MyTypedef)(ButtonPanel*);
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