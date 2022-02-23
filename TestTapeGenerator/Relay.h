#pragma once

class Relay
{
private:
    //Disables copy constructor
    //Relay(const Relay&) = delete;
    //Relay(Relay&) = delete;
    //Disables assignment operator
    //Relay& operator=(const Relay&) = delete;
    //Relay& operator=(Relay&) = delete;
protected:
    bool enabled;
    const uint8_t pin;
    bool reverse;
    const uint8_t relay_delay;

public:
    Relay(const uint8_t pin_, bool reverse_ = false, const uint8_t relay_delay_ = 100) : pin(pin_), reverse(reverse_), relay_delay(relay_delay_)
    {
        pinMode(pin, OUTPUT);
        enabled = true;
        digitalWrite(pin, reverse ? LOW : HIGH);
    }

    void Enable(void)
    {
        if (!enabled) {
            enabled = true;
            digitalWrite(pin, reverse ? LOW : HIGH);
            delay(relay_delay);
        }
    }

    void Disable(void)
    {
        if (enabled) {
            enabled = false;
            digitalWrite(pin, reverse ? HIGH : LOW);
            delay(relay_delay);
        }
    }

    bool IsEnabled(void)
    {
        return enabled;
    }
};