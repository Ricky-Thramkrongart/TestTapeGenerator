#pragma once

#include <Arduino.h>

class Relay
{

private:
    Relay(const Relay&) = delete;
    Relay& operator=(const Relay&) = delete;

protected:
    bool enabled;
    const uint8_t pin;
    bool reverse;
    const uint8_t relay_delay;

public:
    Relay(const uint8_t pin_, bool reverse_ = false, const uint8_t relay_delay_ = 100);
    void Enable(void);
    void Disable(void);
    bool IsEnabled(void);

};