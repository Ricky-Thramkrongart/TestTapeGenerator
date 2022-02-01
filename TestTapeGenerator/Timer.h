#pragma once

template <class T>
class Timer
{
    long ms;
    T* t;
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
    Timer(T* t_) : t(t_), enabled(false), ms(0), period(0) {
    }

    typedef void (T::* TimerTypedef)(Timer*);
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